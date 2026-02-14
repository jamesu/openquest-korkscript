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

ConsoleFunctionValue(spawnFiber, 2, 20, "func, ...")
{
   SimFiberManager::ScheduleInfo initialInfo = {};
   initialInfo.waitMode = SimFiberManager::WAIT_IGNORE;
   KorkApi::FiberId fiberId = gFiberManager->spawnFiber(NULL, argc-1, argv+1, initialInfo);
   
   if (vmPtr->getFiberState(fiberId) < KorkApi::FiberRunResult::State::ERROR)
   {
      return KorkApi::ConsoleValue::makeUnsigned(fiberId);
   }
   else
   {
      return KorkApi::ConsoleValue();
   }
}

ConsoleFunctionValue(throwFiber, 3, 3, "value, soft")
{
   vmPtr->throwFiber((vmPtr->valueAsInt(argv[1]) | vmPtr->valueAsInt(argv[2])) ? BIT(31) : 0);
}


END_SW_NS


