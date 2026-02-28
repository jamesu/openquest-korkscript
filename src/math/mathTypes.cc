//-----------------------------------------------------------------------------
// Copyright (c) 2025-2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 

#include "console/consoleTypes.h"
#include "console/console.h"
#include "math/mPoint.h"
#include "math/mRect.h"
#include "math/mathTypes.h"
#include "math/mPointTypeTraits.h"

namespace KorkApi
{
TypeStorageInterface CreateExprEvalReturnTypeStorage(KorkApi::VmInternal* vmInternal, U32 minSize, U16 typeId);
TypeStorageInterface CreateRegisterStorageFromArgs(KorkApi::VmInternal* vmInternal, U32 argc, KorkApi::ConsoleValue* argv);
}



template <>
struct PointTraits<Point2F>
{
   static constexpr int  N      = 2;
   
   static S32* typeIdPtr() { return &TypePoint2F; }

   static void zero(Point2F& p) { p = Point2F(0,0); }

   static F32& at(Point2F& p, int i)
   {
      switch (i) { case 0: return p.x; default: return p.y; }
   }

   static const char* scanFmt()  { return "%g %g"; }
   static const char* printFmt() { return "%.9g %.9g"; }
};

template <>
struct PointTraits<Point2I>
{
   static constexpr int  N      = 2;
   
   static S32* typeIdPtr() { return &TypePoint2I; }

   static void zero(Point2I& p) { p = Point2I(0,0); }

   static S32& at(Point2I& p, int i)
   {
      switch (i) { case 0: return p.x; default: return p.y; }
   }

   static const char* scanFmt()  { return "%i %i"; }
   static const char* printFmt() { return "%i %i"; }
};

template <>
struct PointTraits<Point3F>
{
   static constexpr int  N      = 3;
   
   static S32* typeIdPtr() { return &TypePoint3F; }

   static void zero(Point3F& p) { p = Point3F(0,0,0); }

   static F32& at(Point3F& p, int i)
   {
      switch (i) { case 0: return p.x; case 1: return p.y; default: return p.z; }
   }

   static const char* scanFmt()  { return "%g %g %g"; }
   static const char* printFmt() { return "%.9g %.9g %.9g"; }
};

template <>
struct PointTraits<Point3I>
{
   static constexpr int  N      = 3;
   
   static S32* typeIdPtr() { return &TypePoint3I; }

   static void zero(Point3I& p) { p = Point3I(0,0,0); }

   static S32& at(Point3I& p, int i)
   {
      switch (i) { case 0: return p.x; case 1: return p.y; default: return p.z; }
   }

   static const char* scanFmt()  { return "%i %i %i"; }
   static const char* printFmt() { return "%i %i %i"; }
};

template <>
struct PointTraits<Point4F>
{
   static constexpr int  N      = 4;
   
   static S32* typeIdPtr() { return &TypePoint4F; }

   static void zero(Point4F& p) { p = Point4F(0,0,0,0); }

   static F32& at(Point4F& p, int i)
   {
      switch (i) { case 0: return p.x; case 1: return p.y; case 2: return p.z; default: return p.w; }
   }

   static const char* scanFmt()  { return "%g %g %g %g"; }
   static const char* printFmt() { return "%.9g %.9g %.9g %.9g"; }
};

template <>
struct PointTraits<Point4I>
{
   static constexpr int  N      = 4;
   
   static S32* typeIdPtr() { return &TypePoint4I; }

   static void zero(Point4I& p) { p = Point4I(0,0,0,0); }

   static S32& at(Point4I& p, int i)
   {
      switch (i) { case 0: return p.x; case 1: return p.y; case 2: return p.z; default: return p.w; }
   }

   static const char* scanFmt()  { return "%i %i %i %i"; }
   static const char* printFmt() { return "%i %i %i %i"; }
};

template <>
struct PointTraits<RectI>
{
   static constexpr int  N      = 4;
   
   static S32* typeIdPtr() { return &TypeRectI; }

   static void zero(RectI& p) { p = RectI(Point2I(0,0), Point2I(0,0)); }

   static S32& at(RectI& p, int i)
   {
      switch (i) { case 0: return p.point.x; case 1: return p.point.y; case 2: return p.extent.x; default: return p.extent.y; }
   }

   static const char* scanFmt()  { return "%i %i %i %i %i %i %i %i"; }
   static const char* printFmt() { return "%i %i %i %i %i %i %i %i"; }
};

template <>
struct PointTraits<RectF>
{
   static constexpr int  N      = 4;
   
   static S32* typeIdPtr() { return &TypeRectF; }

   static void zero(RectF& p) { p = RectF(Point2F(0,0), Point2F(0,0)); }

   static F32& at(RectF& p, int i)
   {
      switch (i) { case 0: return p.point.x; case 1: return p.point.y; case 2: return p.extent.x; default: return p.extent.y; }
   }

   static const char* scanFmt()  { return "%i %i %i %i %i %i %i %i"; }
   static const char* printFmt() { return "%i %i %i %i %i %i %i %i"; }
};



ConsoleType( Point2I, TypePoint2I, sizeof(Point2I), sizeof(Point2I), "" )
ConsoleType( Point2F, TypePoint2F, sizeof(Point2F), sizeof(Point2F), "" )
ConsoleType( Point3F, TypePoint3F, sizeof(Point3F), sizeof(Point3F), "" )
ConsoleType( Point4F, TypePoint4F, sizeof(Point4F), sizeof(Point4F), "" )
ConsoleType( RectI, TypeRectI, sizeof(RectI), sizeof(RectI), "" )
ConsoleType( RectF, TypeRectF, sizeof(RectF), sizeof(RectF), "" )

ConsoleGetType( TypePoint2I )
{
   return getPointDataImpl<Point2I>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

ConsoleGetType( TypePoint2F )
{
   return getPointDataImpl<Point2F>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

ConsoleGetType( TypePoint3F )
{
   return getPointDataImpl<Point3F>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

ConsoleGetType( TypePoint4F )
{
   return getPointDataImpl<Point4F>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

ConsoleGetType( TypeRectI )
{
   return getPointDataImpl<RectI>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

ConsoleGetType( TypeRectF )
{
   return getPointDataImpl<RectF>(vmPtr, inputStorage, outputStorage, fieldUserPtr, flag, requestedType);
}

template <class PointT, class SplatFn>
static KorkApi::ConsoleValue performPointOpImpl(
   KorkApi::Vm* vm,
   U32 op,
   KorkApi::ConsoleValue lhs,
   KorkApi::ConsoleValue rhs,
   const S32* typeIdPtr,
   SplatFn&& splat) // void splat(PointT& out, F32 s)
{
   PointT* lp = (lhs.typeId == (U32)*typeIdPtr) ? (PointT*)lhs.evaluatePtr(vm->getAllocBase()) : nullptr;
   PointT* rp = (rhs.typeId == (U32)*typeIdPtr) ? (PointT*)rhs.evaluatePtr(vm->getAllocBase()) : nullptr;

   // Your original assumes at least one side is a point.
   if (!lp && !rp)
      return lhs;

   PointT a{}, b{};
   PointT* outPtr = nullptr;
   bool returnLhs = true;

   if (!lp)
   {
      splat(a, vm->valueAsFloat(lhs));
      b = *rp;
      outPtr = rp;
      returnLhs = false;
   }
   else if (!rp)
   {
      a = *lp;
      splat(b, vm->valueAsFloat(rhs));
      outPtr = lp;
      returnLhs = true;
   }
   else
   {
      a = *lp;
      b = *rp;
      outPtr = lp;     // keep original policy: mutate lhs storage
      returnLhs = true;
   }

   using namespace Compiler;
   switch (op)
   {
      case OP_ADD: *outPtr = a + b; break;
      case OP_SUB: *outPtr = a - b; break;
      case OP_MUL: *outPtr = a * b; break;
      case OP_DIV: *outPtr = a / b; break;
      case OP_NEG: *outPtr = -a;    break;
      default: break;
   }

   return returnLhs ? lhs : rhs;
}

ConsoleTypeOp(TypePoint2I)
{
   auto splat = [](Point2I& p, S32 s) { p = Point2I(s,s); };
   return performPointOpImpl<Point2I>(vmPtr, op, lhs, rhs, &TypePoint2I, splat);
}

ConsoleTypeOp(TypePoint2F)
{
   auto splat = [](Point2F& p, F32 s) { p = Point2F(s,s); };
   return performPointOpImpl<Point2F>(vmPtr, op, lhs, rhs, &TypePoint2F, splat);
}

ConsoleTypeOp(TypePoint3F)
{
   auto splat = [](Point3F& p, F32 s) { p = Point3F(s,s,s); };
   return performPointOpImpl<Point3F>(vmPtr, op, lhs, rhs, &TypePoint3F, splat);
}

ConsoleTypeOp(TypePoint4F)
{
   auto splat = [](Point4F& p, F32 s) { p = Point4F(s,s,s,s); };
   return performPointOpImpl<Point4F>(vmPtr, op, lhs, rhs, &TypePoint4F, splat);
}

ConsoleTypeOpDefault(TypeRectI)
ConsoleTypeOpDefault(TypeRectF)

