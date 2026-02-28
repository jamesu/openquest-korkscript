#pragma once

template <class PointT>
struct PointTraits;


template <class PointT, class Traits = PointTraits<PointT>>
static bool getPointDataImpl(
   KorkApi::Vm* vmPtr,
   KorkApi::TypeStorageInterface* inputStorage,
   KorkApi::TypeStorageInterface* outputStorage,
                             void* fieldUserPtr,
   BitSet32 flag,
   U32 requestedType)
{
   const KorkApi::ConsoleValue* argv = nullptr;
   U32 argc = inputStorage ? inputStorage->data.argc : 0;
   bool directLoad = false;

   if (argc > 0 && inputStorage->data.storageRegister)
   {
      argv = inputStorage->data.storageRegister;
   }
   else
   {
      argc = 1;
      argv = &inputStorage->data.storageAddress;
      directLoad = true;
   }

   PointT v;
   Traits::zero(v);

   // --- input ---
   if (inputStorage->isField && directLoad)
   {
      const PointT* src = (const PointT*)inputStorage->data.storageAddress
                             .evaluatePtr(vmPtr->getAllocBase());
      if (!src) return false;
      v = *src;
   }
   else
   {
      if (argc == Traits::N)
      {
         for (int i = 0; i < Traits::N; ++i)
         {
            Traits::at(v, i) = (F32)argv[i].getFloat((F64)argv[i].getInt(0));
         }
      }
      else if (argc == 1)
      {
         if (argv[0].typeId == *Traits::typeIdPtr())
         {
            const PointT* src = (const PointT*)argv[0].evaluatePtr(vmPtr->getAllocBase());
            if (src) v = *src;
         }
         else
         {
            const char* s = vmPtr->valueAsString(argv[0]);
            if (!s) s = "";

            // Generic scanf path (expects scanFmt matches Traits::N)
            if constexpr (Traits::N == 2)
               sscanf(s, Traits::scanFmt(), &Traits::at(v,0), &Traits::at(v,1));
            else if constexpr (Traits::N == 3)
               sscanf(s, Traits::scanFmt(), &Traits::at(v,0), &Traits::at(v,1), &Traits::at(v,2));
            else if constexpr (Traits::N == 4)
               sscanf(s, Traits::scanFmt(), &Traits::at(v,0), &Traits::at(v,1), &Traits::at(v,2), &Traits::at(v,3));
            else if constexpr (Traits::N == 8) // rect types
               sscanf(s, Traits::scanFmt(), &Traits::at(v,0), &Traits::at(v,1), &Traits::at(v,2), &Traits::at(v,3), &Traits::at(v,4), &Traits::at(v,5), &Traits::at(v,6), &Traits::at(v,7));
            else
               return false;
         }
      }
      else
      {
         return false;
      }
   }

   // --- output ---
   auto finalizeAndPushRegister = [&]() {
      if (outputStorage->data.storageRegister)
         *outputStorage->data.storageRegister = outputStorage->data.storageAddress;
      return true;
   };

   if (requestedType == *Traits::typeIdPtr())
   {
      PointT* dstPtr = (PointT*)outputStorage->data.storageAddress
                          .evaluatePtr(vmPtr->getAllocBase());
      if (!dstPtr) return false;

      *dstPtr = v;
      return finalizeAndPushRegister();
   }
   else if (requestedType == KorkApi::ConsoleValue::TypeInternalString)
   {
      constexpr U32 bufLen = 32 * Traits::N;
      outputStorage->FinalizeStorage(outputStorage, bufLen);

      char* out = (char*)outputStorage->data.storageAddress.evaluatePtr(vmPtr->getAllocBase());
      if (!out) return false;

      if constexpr (Traits::N == 2)
         snprintf(out, bufLen, Traits::printFmt(), Traits::at(v,0), Traits::at(v,1));
      else if constexpr (Traits::N == 3)
         snprintf(out, bufLen, Traits::printFmt(), Traits::at(v,0), Traits::at(v,1), Traits::at(v,2));
      else if constexpr (Traits::N == 4)
         snprintf(out, bufLen, Traits::printFmt(), Traits::at(v,0), Traits::at(v,1), Traits::at(v,2), Traits::at(v,3));
      else if constexpr (Traits::N == 8)
         snprintf(out, bufLen, Traits::printFmt(), Traits::at(v,0), Traits::at(v,1), Traits::at(v,2), Traits::at(v,3), Traits::at(v,4), Traits::at(v,5), Traits::at(v,6), Traits::at(v,7));
      else
         return false;

      return finalizeAndPushRegister();
   }
   else
   {
      // Cast through VM using N numeric values
      KorkApi::ConsoleValue vals[Traits::N];
      for (int i = 0; i < Traits::N; ++i)
      {
         vals[i] = KorkApi::ConsoleValue::makeNumber(Traits::at(v, i));
      }
      
      KorkApi::TypeStorageInterface castInput;;
      vmPtr->initRegisterTypeStorage(Traits::N, vals, &castInput);

      return vmPtr->castValue(requestedType, &castInput, outputStorage, fieldUserPtr, flag);
   }
}
