#include "engine.h"

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//

std::vector<ITickable::TickableInfo> ITickable::smTickList;


void ITickable::registerTickable()
{
  TickableInfo info = {};
  info.tickable = this;
  info.registered = true;
  smTickList.push_back(info);
}

void ITickable::unregisterTickable()
{
  auto itr = std::find_if(smTickList.begin(), smTickList.end(), [this](const TickableInfo& info){
     return info.tickable == this;
  });
  
  if (itr != smTickList.end())
  {
     itr->registered = false;
  }
}

void ITickable::doFixedTick(F32 dt)
{
  for (TickableInfo info : smTickList)
  {
     info.tickable->onFixedTick(dt);
  }
  
  smTickList.erase(
      std::remove_if(smTickList.begin(), smTickList.end(),
          [](const TickableInfo& info) {
              return !info.registered;
          }),
                   smTickList.end()
  );
}


BEGIN_SW_NS


IMPLEMENT_CONOBJECT(SentenceQueueManager);

ConsoleFunctionValue(yieldFiber, 2, 2, "value")
{
   vmPtr->suspendCurrentFiber();
   return argv[1]; // NOTE: this will be set as yield value
}

ConsoleFunctionValue(breakFiber, 1, 1, "")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = 0;
   sp.minTime = gFiberManager->getCurrentTick() + 1;
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_TICK, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(delayFiber, 2, 2, "ticks")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = 0;
   sp.minTime = gFiberManager->getCurrentTick() + vmPtr->valueAsInt(argv[1]);
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_TICK, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(spawnFiber, 3, 20, "flagMask, func, ...")
{
   SimFiberManager::ScheduleInfo initialInfo = {};
   initialInfo.waitMode = SimFiberManager::WAIT_REMOVE;
   initialInfo.param.flagMask = (U64)vmPtr->valueAsInt(argv[1]);
   KorkApi::FiberId fiberId = gFiberManager->spawnFiber(NULL, argc-2, argv+2, initialInfo);
   
   if (vmPtr->getFiberState(fiberId) < KorkApi::FiberRunResult::State::ERROR)
   {
      return KorkApi::ConsoleValue::makeUnsigned(fiberId);
   }
   else
   {
      return KorkApi::ConsoleValue();
   }
}

ConsoleMethodValue(SimObject, spawnFiber, 4, 20, "flagMask, func, ...")
{
   SimFiberManager::ScheduleInfo initialInfo = {};
   initialInfo.waitMode = SimFiberManager::WAIT_REMOVE; // should be in this state so can be cleaned up if no wait
   initialInfo.param.flagMask = (U64)vmPtr->valueAsInt(argv[1]);
   
   // params are:       spawnFiber, obj, func, ...
   // params should be: func, obj, ...
   std::array<KorkApi::ConsoleValue, 20> params;
   params[0] = argv[3];
   params[1] = argv[2];
   if (argc > 4)
   {
      std::copy(argv+4, argv+4+(argc-4), params.begin()+2);
   }
   
   KorkApi::FiberId fiberId = gFiberManager->spawnFiber(object, argc-2, params.data(), initialInfo);
   
   if (vmPtr->getFiberState(fiberId) < KorkApi::FiberRunResult::State::ERROR)
   {
      return KorkApi::ConsoleValue::makeUnsigned(fiberId);
   }
   else
   {
      return KorkApi::ConsoleValue();
   }
}

ConsoleFunctionValue(isFiberRunning, 2, 2, "fiberId")
{
   KorkApi::FiberId fiberId = (KorkApi::FiberId)vmPtr->valueAsInt(argv[1]);
   KorkApi::FiberRunResult::State state = vmPtr->getFiberState(fiberId);
   return KorkApi::ConsoleValue::makeUnsigned(state == KorkApi::FiberRunResult::State::SUSPENDED);
}

ConsoleFunctionValue(stopFiber, 2, 2, "fiberId")
{
   KorkApi::FiberId fiberId = (KorkApi::FiberId)vmPtr->valueAsInt(argv[1]);
   gFiberManager->cleanupFiber(fiberId);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(throwFiber, 3, 3, "value, soft")
{
   U32 catchMask = vmPtr->valueAsInt(argv[1]) | (vmPtr->valueAsInt(argv[2]) ? BIT(31) : 0);
   vmPtr->throwFiber(catchMask);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(throwFibersWithMask, 4, 4, "fiberMask, catchMask, soft")
{
   U32 catchMask = vmPtr->valueAsInt(argv[2]) | (vmPtr->valueAsInt(argv[3]) ? BIT(31) : 0);
   gFiberManager->throwWithMask(vmPtr->valueAsInt(argv[1]), catchMask);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(throwFibersWithObject, 4, 4, "objectId, catchMask, soft")
{
   U32 catchMask = vmPtr->valueAsInt(argv[2]) | (vmPtr->valueAsInt(argv[3]) ? BIT(31) : 0);
   gFiberManager->throwWithObject(vmPtr->valueAsInt(argv[1]), catchMask);
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(pushFiberSuspendFlags, 2, 2, "fiberMask")
{
   gFiberManager->pushFiberSuspendFlags((U64)vmPtr->valueAsInt(argv[1]));
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(popFiberSuspendFlags, 1, 1, "")
{
   gFiberManager->popFiberSuspendFlags();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(waitForMessage, 1, 1, "")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = SCHEDULE_FLAG_MESSAGE;
   sp.minTime = 0;
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_FLAGS_CLEAR, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(waitForCamera, 1, 1, "")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = SCHEDULE_FLAG_CAMERA_MOVING;
   sp.minTime = 0;
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_FLAGS_CLEAR, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(waitForSentence, 1, 1, "")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = SCHEDULE_FLAG_SENTENCE_BUSY;
   sp.minTime = 0;
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_FLAGS_CLEAR, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(waitForFiber, 2, 2, "")
{
   SimFiberManager::ScheduleParam sp;
   sp.flagMask = 0;
   sp.minTime = vmPtr->valueAsInt(argv[1]);
   gFiberManager->setFiberWaitMode(vmPtr->getCurrentFiber(), SimFiberManager::WAIT_FIBER, sp);
   vmPtr->suspendCurrentFiber();
   return KorkApi::ConsoleValue();
}


SentenceQueueManager::SentenceQueueManager()
{
   mLastFiber = 0;
}

bool SentenceQueueManager::pushItem(SimObjectId verb, SimObjectId objA, SimObjectId objB)
{
   if (mSentences.size() >= MaxSentences)
   {
      return false;
   } 

   SentenceQueueItem item;
   item.verb = verb;
   item.objA = objA;
   item.objB = objB;
   mSentences.push_back(item);
   return true;
}

void SentenceQueueManager::execItem()
{
   // Wait until last fiber has stopped running
   if (isBusy())
   {
      return;
   }

   cancel();

   if (!mSentences.empty())
   {
      SentenceQueueItem lastItem = mSentences.front();
      mSentences.erase(mSentences.begin());

      VerbDisplay* verbObject = nullptr;
      DisplayBase* objA = nullptr;
      DisplayBase* objB = nullptr;
      
      Sim::findObject(lastItem.verb, verbObject);
      Sim::findObject(lastItem.objA, objA);
      Sim::findObject(lastItem.objB, objB);

      if (lastItem.verb && lastItem.objA)
      {
         char verbFunc[64];
         snprintf(verbFunc, 64, "on%s", verbObject->getInternalName());

         KorkApi::ConsoleValue rv = Con::executef(objA, 
                       "spawnFiber", 
                       KorkApi::ConsoleValue::makeUnsigned(SCHEDULE_FLAG_IS_SENTENCE_HANDLER),
                       KorkApi::ConsoleValue::makeString(verbFunc), 
                       KorkApi::ConsoleValue::makeUnsigned(lastItem.objA), 
                       KorkApi::ConsoleValue::makeUnsigned(lastItem.objB));

         mLastFiber = (KorkApi::FiberId)getVM()->valueAsInt(rv);
      }
   }
}

bool SentenceQueueManager::isBusy()
{
   auto state = getVM()->getFiberState(mLastFiber);
   return (state == KorkApi::FiberRunResult::State::SUSPENDED || 
           state == KorkApi::FiberRunResult::State::RUNNING);
}

void SentenceQueueManager::cancel()
{
   gFiberManager->cleanupFiber(mLastFiber);
   mLastFiber = 0;
}

ConsoleMethodValue(SentenceQueueManager, push, 5, 5, "(verb, objA, objB)")
{
   VerbDisplay* verbObject = nullptr;
   DisplayBase* objA = nullptr;
   DisplayBase* objB = nullptr;

   Sim::findObject(argv[2], verbObject);
   Sim::findObject(argv[3], objA);
   Sim::findObject(argv[4], objB);

   if (verbObject && objA)
   {
      object->pushItem(verbObject->getId(),
         objA->getId(),
         objB ? objB->getId() : 0);
   }
   return KorkApi::ConsoleValue();
}

ConsoleMethodValue(SentenceQueueManager, isBusy, 2, 2, "")
{
   return KorkApi::ConsoleValue::makeUnsigned(object->isBusy());
}

ConsoleMethodValue(SentenceQueueManager, cancel, 2, 2, "")
{
   object->cancel();
   return KorkApi::ConsoleValue();
}

ConsoleFunctionValue(nop, 1, 1, "")
{
   Con::printf("NOP");
   return KorkApi::ConsoleValue();
}


END_SW_NS


