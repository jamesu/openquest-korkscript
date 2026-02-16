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


ConsoleFunctionValue(yieldFiber, 2, 2, "value")
{
   vmPtr->suspendCurrentFiber();
   return argv[1]; // NOTE: this will be set as yield value
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
   initialInfo.waitMode = SimFiberManager::WAIT_IGNORE;
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
   initialInfo.waitMode = SimFiberManager::WAIT_IGNORE;
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
   vmPtr->throwFiber((vmPtr->valueAsInt(argv[1]) | vmPtr->valueAsInt(argv[2])) ? BIT(31) : 0);
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


END_SW_NS


