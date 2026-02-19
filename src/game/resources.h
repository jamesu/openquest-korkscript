#pragma once

//-----------------------------------------------------------------------------
// Copyright (c) 2026 James S Urquhart
// See AUTHORS file and git repository for contributor information.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//


struct BoxInfo
{
   enum
   {
      NumScaleSlots = 5
   };
   
   enum BoxFlags
   {
      BOXF_NONE = 0,
      
      BOXF_DISABLED = BIT(0),
      BOXF_UNKNOWN1 = BIT(1),
      BOXF_UNKNOWN2 = BIT(2),
      
      BOXF_XFLIP = BIT(3),
      BOXF_YFLIP = BIT(4),
      BOXF_IGNORE_SCALE = BIT(5),
      BOXF_LOCKED = BIT(6),
      BOXF_INVISIBLE = BIT(7)
   };
   
   struct Box
   {
      std::string name;
      uint16_t startPoint;
      uint16_t numPoints;
      uint16_t scale;
      uint8_t mask;
      uint8_t flags;
   };
   
   struct ScaleInfo
   {
      U16 sy1[2];
      U16 sy2[2];
   };
   
   std::vector<Box> boxes;
   std::vector<Point2I> points;
   std::vector<ScaleInfo> scaleBands;
   std::vector<U8> nextBoxHopList;
   
   static inline bool OnSegment(const Point2I& a, const Point2I& b, const Point2I& p)
   {
      return std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x) &&
      std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
   }
   
   static inline bool PointOnEdge(Point2I* poly, U32 n, Point2I p)
   {
      for (int i = 0; i < n; i++) {
         const Point2I& a = poly[i];
         const Point2I& b = poly[(i + 1) % n];
         S32 c = mCross(a, b, p);
         if (c == 0 && OnSegment(a, b, p)) return true;
      }
      return false;
   }
   
   struct AdjustBoxResult
   {
      Point2I pos;
      int box;
      bool inside;
      S32 bestD2;
   };
   
   static Point2I PolygonCentroid(const Point2I* poly, U32 n) {
      // Standard polygon centroid formula; returns rounded integer.
      // If area is near zero (degenerate), falls back to vertex average.
      double cx = 0.0, cy = 0.0;
      double a2 = 0.0; // actually 2*area
      
      for (U32 i = 0; i < n; ++i) {
         const Point2I& p = poly[i];
         const Point2I& q = poly[(i + 1) % n];
         const double cross = double(p.x) * double(q.y) - double(q.x) * double(p.y);
         a2 += cross;
         cx += (double(p.x) + double(q.x)) * cross;
         cy += (double(p.y) + double(q.y)) * cross;
      }
      
      if (std::abs(a2) < 1e-9) {
         // Degenerate polygon: fallback to average of vertices
         S32 sx = 0, sy = 0;
         for (U32 i = 0; i < n; ++i) { sx += poly[i].x; sy += poly[i].y; }
         return Point2I{ int(sx / n), int(sy / n) };
      }
      
      cx /= (3.0 * a2);
      cy /= (3.0 * a2);
      return Point2I{ int(std::lround(cx)), int(std::lround(cy)) };
   }
   
   Point2I GetBoxCenter(int bid)
   {
      const Box& box = boxes[bid];
      Point2I* boxPoints = points.data() + (box.startPoint);
      return PolygonCentroid(boxPoints, box.numPoints);
   }
   
   bool FindContainingBox(Point2I dst,
                          bool (*isSelectable)(const Box&),
                          bool preferHigherIndex,
                          S32 threshold,
                          AdjustBoxResult& outResult
                          ) {
      const S32 nBoxes = U32(boxes.size());
      const S32 begin = preferHigherIndex ? (nBoxes - 1) : 0;
      const S32 end   = preferHigherIndex ? -1 : nBoxes;
      const S32 step  = preferHigherIndex ? -1 : 1;
      
      for (S32 i = begin; i != end; i += step)
      {
         const Box& b = boxes[i];
         
         if (!isSelectable(b) ||
             b.numPoints < 3 ||
             b.startPoint + b.numPoints > points.size())
         {
            continue;
         }
         
         const Point2I* poly = &points[b.startPoint];
         const U32 n = b.numPoints;
         
         if (threshold > 0)
         {
            RectI aabb = PolyAABB(poly, n);
            if (AABBQuickReject(aabb, dst, threshold))
            {
               continue;
            }
         }
         
         if (PointInConvexPoly((Point2I*)poly, n, dst))
         {
            outResult.pos = dst;
            outResult.box = i;
            outResult.inside = true;
            outResult.bestD2 = 0;
            return true;
         }
      }
      
      return false;
   }
   
   bool FindNearestBoxAndSnapPoint(Point2I dst,
                                   bool (*isSelectable)(const Box&),
                                   bool preferHigherIndex,
                                   int threshold,
                                   AdjustBoxResult& outResult
                                   )
   {
      S32 bestBox = -1;
      Point2I bestPt = dst;
      S32 bestD2 = std::numeric_limits<S32>::max();
      
      const S32 nBoxes = S32(boxes.size());
      const S32 begin = preferHigherIndex ? (nBoxes - 1) : 0;
      const S32 end   = preferHigherIndex ? -1 : nBoxes;
      const S32 step  = preferHigherIndex ? -1 : 1;
      
      for (S32 i = begin; i != end; i += step)
      {
         const Box& b = boxes[i];
         if (!isSelectable(b) ||
             b.numPoints < 3 ||
             b.startPoint + b.numPoints > points.size())
         {
            continue;
         }
         
         const Point2I* poly = &points[b.startPoint];
         const U32 n = b.numPoints;
         
         if (threshold > 0)
         {
            RectI aabb = PolyAABB(poly, n);
            if (AABBQuickReject(aabb, dst, threshold))
            {
               continue;
            }
         }
         
         Point2I q = ClosestPointOnConvexPolyEdges(poly, n, dst);
         S32 d2 = (q - dst).lenSquared();
         
         if (d2 < bestD2)
         {
            bestD2 = d2;
            bestBox = i;
            bestPt = q;
         }
      }
      
      if (bestBox == -1)
      {
         return false;
      }
      
      outResult.pos = bestPt;
      outResult.box = bestBox;
      outResult.inside = false;
      outResult.bestD2 = bestD2;
      return true;
   }
   
   static inline Point2I ClosestPointOnSegment(Point2I a, Point2I b, Point2I p, bool* inSlab = nullptr)
   {
      const F32 ax = F32(a.x), ay = F32(a.y);
      const F32 bx = F32(b.x), by = F32(b.y);
      const F32 px = F32(p.x), py = F32(p.y);
      
      const F32 abx = bx - ax, aby = by - ay;
      const F32 apx = px - ax, apy = py - ay;
      
      const F32 denom = abx * abx + aby * aby;
      F32 t = 0.0f;
      if (denom > 0.0f) t = (apx * abx + apy * aby) / denom;
      
      if (t < 0.0f)      {if (inSlab) *inSlab = false; t = 0.0f;}
      else if (t > 1.0f) {if (inSlab) *inSlab = false; t = 1.0f;}
      
      return Point2I{ int(std::lround(ax + t * abx)), int(std::lround(ay + t * aby)) };
   }
   
   static inline RectI PolyAABB(const Point2I* poly, U32 n)
   {
      Point2I mn = poly[0];
      Point2I mx = poly[0];
      for (U32 i = 1; i < n; ++i)
      {
         mn.x = std::min(mn.x, poly[i].x);
         mn.y = std::min(mn.y, poly[i].y);
         mx.x = std::max(mx.x, poly[i].x);
         mx.y = std::max(mx.y, poly[i].y);
      }
      return RectI(mn, mx - mn);
   }
   
   static inline bool AABBQuickReject(const RectI rect, Point2I p, S32 threshold)
   {
      const Point2I& mn = rect.point;
      const Point2I& mx = rect.point + rect.extent;
      if (p.x < mn.x - threshold) return true;
      if (p.x > mx.x + threshold) return true;
      if (p.y < mn.y - threshold) return true;
      if (p.y > mx.y + threshold) return true;
      return false;
   }
   
   static inline Point2I ClosestPointOnConvexPolyEdges(const Point2I* poly, U32 n, Point2I p)
   {
      Point2I best = poly[0];
      S32 bestD2 = std::numeric_limits<S32>::max();
      
      for (U32 i = 0; i < n; ++i)
      {
         Point2I a = poly[i];
         Point2I b = poly[(i + 1) % n];
         Point2I q = ClosestPointOnSegment(a, b, p);
         S32 d2 = (p - q).lenSquared();
         if (d2 < bestD2)
         {
            bestD2 = d2;
            best = q;
         }
      }
      return best;
   }
   
   static bool PointInConvexPoly(Point2I* poly, U32 n, Point2I p)
   {
      if (n < 3)
      {
         return false;
      }
      
      if (PointOnEdge(poly, n, p))
      {
         return true;
      }
      
      int sign = 0;
      for (int i = 0; i < n; i++)
      {
         const Point2I& a = poly[i];
         const Point2I& b = poly[(i + 1) % n];
         S32 c = mCross(a, b, p);
         if (c == 0) continue;
         int s = (c > 0) ? 1 : -1;
         if (sign == 0) sign = s;
         else if (sign != s) return false;
      }
      return true;
   }
   
   F32 evalScale(Point2I roomPos)
   {
      F32 outScale = 1.0f;
      U16 realRoomPos = std::clamp(roomPos.x, 0, 65536);
      evalBandScale(realRoomPos, outScale);
      evalBoxScale(roomPos, outScale); // may override
      return outScale;
   }
   
   void evalBoxScale(Point2I roomPos, F32& outScale)
   {
      for (Box& box : boxes)
      {
         if (PointInConvexPoly(points.data() + box.startPoint, box.numPoints, roomPos))
         {
            if (box.flags & BOXF_DISABLED)
            {
               continue;
            }
            
            if (box.flags & BOXF_IGNORE_SCALE)
            {
               outScale = 1.0;
            }
            else if ((box.scale & 0x8000) != 0)
            {
               U16 trueScale = box.scale & 0x7;
               U16 realRoomPos = std::clamp(roomPos.x, 0, 65536);
               outScale = evalScaleForBand(scaleBands[trueScale], realRoomPos);
            }
            else if (box.scale > 0)
            {
               outScale = scaleToFloat(box.scale);
            }
            
            return;
         }
      }
   }
   
   inline F32 evalScaleForBand(ScaleInfo& info, S16 roomPos)
   {
      F32 startScale = scaleToFloat(info.sy1[0]);
      F32 endScale = scaleToFloat(info.sy2[0]);
      
      U16 relPos = roomPos - info.sy1[1];
      U16 bandSize = info.sy2[1] - info.sy1[1];
      
      F32 ratioInBand = (F32)(relPos) / (F32) bandSize;
      return startScale + (ratioInBand * (endScale - startScale));
   }
   
   F32 evalBandScale(U16 roomPos, F32 outScale)
   {
      if (scaleBands.empty())
      {
         return 1.0f;
      }
      
      for (ScaleInfo& info : scaleBands)
      {
         if (roomPos >= info.sy1[1] && roomPos < info.sy2[1])
         {
            return evalScaleForBand(info, roomPos);
         }
      }
      
      return 1.0f;
   }
   
   struct EdgeRef {
      int boxId;
      int edgeIdx;      // edge is points[i] -> points[i+1]
      Point2I a, b;     // endpoints
   };
   
   struct PortalEdgePair {
      EdgeRef srcEdge;
      EdgeRef dstEdge;
      Point2I midOnSrc; // midpoint of overlap (or closest pair midpoint) mapped onto src edge
      Point2I midOnDst; // same mapped onto dst edge
      bool hasOverlap;  // true if 1D overlap exists along the edge direction
   };
   
   bool IsNeighbor(int a, int b) { return getNextBoxIndex(a, b) == b; }
   
   static inline Point2F UnitDir(const Point2I& a, const Point2I& b)
   {
      F32 dx = F32(b.x - a.x);
      F32 dy = F32(b.y - a.y);
      F32 len = std::sqrt(dx*dx + dy*dy);
      if (len < 1e-6f) { return Point2F(1.0f, 0.0f); }
      
      return Point2F(dx / len, dy / len);
   }
   
   static inline F32 ProjectPoint(const Point2I& p, F32 ux, F32 uy)
   {
      return F32(p.x) * ux + F32(p.y) * uy;
   }
   
   static inline bool Overlap1D(F32 a0, F32 a1, F32 b0, F32 b1, F32& out0, F32& out1)
   {
      if (a0 > a1) std::swap(a0, a1);
      if (b0 > b1) std::swap(b0, b1);
      out0 = std::max(a0, b0);
      out1 = std::min(a1, b1);
      return out0 <= out1;
   }
   
   static S32 ApproxSegSegDist2(Point2I a0, Point2I a1, Point2I b0, Point2I b1)
   {
      Point2I q0 = BoxInfo::ClosestPointOnSegment(b0, b1, a0);
      Point2I q1 = BoxInfo::ClosestPointOnSegment(b0, b1, a1);
      Point2I p0 = BoxInfo::ClosestPointOnSegment(a0, a1, b0);
      Point2I p1 = BoxInfo::ClosestPointOnSegment(a0, a1, b1);
      
      S32 d0 = (a0 - q0).lenSquared();
      S32 d1 = (a1 - q1).lenSquared();
      S32 d2 = (b0 - p0).lenSquared();
      S32 d3 = (b1 - p1).lenSquared();
      
      return std::min(std::min(d0, d1), std::min(d2, d3));
   }
   
   
   static inline Point2I pointAt(Point2I S0, Point2F d, F32 t)
   {
      return Point2I{
         int(std::lround(S0.x + d.x * t)),
         int(std::lround(S0.y + d.y * t))
      };
   };
   
   static inline F32 projT(Point2I S0, F32 len2, Point2F d, Point2I p)
   {
      return ((p.x - S0.x)*d.x + (p.y - S0.y)*d.y) / len2;
   }
   
   bool ComputeOverlapSegment(Point2I S0, Point2I S1,
                              Point2I D0, Point2I D1,
                              Point2I& outA,
                              Point2I& outB)
   {
      Point2F d(F32(S1.x - S0.x), F32(S1.y - S0.y));
      
      F32 len2 = d.lenSquared();
      if (len2 < 1e-6f)
         return false;
      
      F32 s0 = 0.0f;
      F32 s1 = 1.0f;
      
      F32 d0 = projT(S0, len2, d, D0);
      F32 d1 = projT(S0, len2, d, D1);
      
      if (d0 > d1) std::swap(d0, d1);
      
      F32 lo = std::max(s0, d0);
      F32 hi = std::min(s1, d1);
      
      if (lo > hi)
         return false; // no overlap
      
      outA = pointAt(S0, d, lo);
      outB = pointAt(S0, d, hi);
      return true;
   }
   
   PortalEdgePair FindBestPortalEdgePair(int srcBoxId, int dstBoxId, F32 minParallel = 0.92f)
   {
      PortalEdgePair out{};
      out.srcEdge = {srcBoxId, -1, {}, {}};
      out.dstEdge = {dstBoxId, -1, {}, {}};
      out.midOnSrc = Point2I(0,0);
      out.midOnDst = Point2I(0,0);
      out.hasOverlap = false;
      
      const Box& s = boxes[srcBoxId];
      const Box& d = boxes[dstBoxId];
      if (s.numPoints < 2 || d.numPoints < 2) return out;
      
      const Point2I* ps = &points[s.startPoint];
      const Point2I* pd = &points[d.startPoint];
      
      F32 bestScore = std::numeric_limits<F32>::max();
      
      // weights
      const F32 wPar  = 2000.0f;   // penalty weight for not-parallel
      const F32 wDist = 1.0f;      // distance^2 weight
      
      for (U32 i = 0; i < s.numPoints; ++i)
      {
         Point2I s0 = ps[i];
         Point2I s1 = ps[(i + 1) % s.numPoints];
         
         Point2F su = UnitDir(s0, s1);
         
         for (U32 j = 0; j < d.numPoints; ++j) {
            Point2I d0 = pd[j];
            Point2I d1 = pd[(j + 1) % d.numPoints];
            Point2F du = UnitDir(d0, d1);
            
            F32 par = std::fabs(mDot(su, du)); // 1 = parallel
            if (par < minParallel)
            {
               continue;
            }
            
            S32 dist2 = ApproxSegSegDist2(s0, s1, d0, d1);
            
            // score: smaller is better
            F32 score = (1.0f - par) * wPar + F32(dist2) * wDist;
            
            if (score < bestScore) {
               bestScore = score;
               out.srcEdge = {srcBoxId, (int)i, s0, s1};
               out.dstEdge = {dstBoxId, (int)j, d0, d1};
            }
         }
      }
      
      // If nothing met parallel constraint, relax it once (still return something)
      if (out.srcEdge.edgeIdx < 0)
      {
         return FindBestPortalEdgePair(srcBoxId, dstBoxId, 0.0f);
      }
      
      // Compute portal midpoints via 1D overlap (or fallback)
      {
         const Point2I s0 = out.srcEdge.a, s1 = out.srcEdge.b;
         const Point2I d0 = out.dstEdge.a, d1 = out.dstEdge.b;
         
         Point2F u = UnitDir(s0, s1); // use src edge direction as reference
         
         F32 sA = ProjectPoint(s0, u.x, u.y);
         F32 sB = ProjectPoint(s1, u.x, u.y);
         F32 dA = ProjectPoint(d0, u.x, u.y);
         F32 dB = ProjectPoint(d1, u.x, u.y);
         
         F32 o0, o1;
         if (Overlap1D(sA, sB, dA, dB, o0, o1))
         {
            out.hasOverlap = true;
            F32 mid = 0.5f * (o0 + o1);
            
            // Convert that projected coordinate back onto each edge by projecting a point on the line.
            Point2I guessOnLine(
               int(std::lround(F32(s0.x) + u.x * (mid - sA))),
               int(std::lround(F32(s0.y) + u.y * (mid - sA)))
            );
            
            out.midOnSrc = ClosestPointOnSegment(s0, s1, guessOnLine);
            out.midOnDst = ClosestPointOnSegment(d0, d1, guessOnLine);
         }
         else
         {
            // No overlap: corner-touch / near but skew. Use midpoint of closest endpoint projections.
            Point2I qS0 = ClosestPointOnSegment(d0, d1, s0);
            Point2I qS1 = ClosestPointOnSegment(d0, d1, s1);
            
            Point2I bestS = s0;
            Point2I bestD = qS0;
            S32 bestD2 = (s0 - qS0).lenSquared();
            
            S32 d2 = (s1 - qS1).lenSquared();
            if (d2 < bestD2) { bestD2 = d2; bestS = s1; bestD = qS1; }
            
            // also check dst endpoints to src segment
            Point2I qD0 = ClosestPointOnSegment(s0, s1, d0);
            Point2I qD1 = ClosestPointOnSegment(s0, s1, d1);
            d2 = (d0 - qD0).lenSquared();
            if (d2 < bestD2) { bestD2 = d2; bestS = qD0; bestD = d0; }
            d2 = (d1 - qD1).lenSquared();
            if (d2 < bestD2) { bestD2 = d2; bestS = qD1; bestD = d1; }
            
            Point2I mid{ (bestS.x + bestD.x) / 2, (bestS.y + bestD.y) / 2 };
            out.midOnSrc = ClosestPointOnSegment(s0, s1, mid);
            out.midOnDst = ClosestPointOnSegment(d0, d1, mid);
            out.hasOverlap = false;
         }
      }
      
      return out;
   }
   
   
   U8 getNextBoxIndex(int src, int dst)
   {
      if (src < 0)
      {
         return dst < 0 ? 0 : dst;
      }
      return nextBoxHopList[src * boxes.size() + dst];
   }
   
   static F32 scaleToFloat(uint16_t value)
   {
      return value / 100.0f;
   }
   
   void reset()
   {
      boxes.clear();
      points.clear();
      scaleBands.clear();
      nextBoxHopList.clear();
   }
   
   bool read(Stream& stream)
   {
      std::vector<U8> boxmData;
      
      while (stream.getStatus() == Stream::Ok)
      {
         IFFBlock block;
         if (!stream.read(sizeof(IFFBlock), &block))
         {
            return false;
         }
         
         S32 size = convertBEndianToHost(block.getRawSize());
         if (size < 8)
         {
            return false;
         }
         size -= 8;
         
         if (block.ident == 1297633090) // BOXM
         {
            boxmData.resize(size);
            if (!stream.read(size, boxmData.data()))
            {
               return false;
            }
         }
         else if (block.ident == 1685614434 || // boxd
                  block.ident == 1112496196)  // BOXD
         {
            // skip first box on BOXD
            if (block.ident == 1112496196)
            {
               stream.setPosition(stream.getPosition()+22);
            }
            
            if (stream.getStatus() == Stream::Ok)
            {
               char nameBuf[256];
               Box rootBox = {};
               boxes.push_back(rootBox);
               
               while (size >= 21)
               {
                  Box outBox = {};
                  U8 sz = 0;
                  stream.read(&sz);
                  stream.read(sz, nameBuf);
                  nameBuf[sz] = '\0';
                  outBox.name = nameBuf;
                  size -= sz + 1;
                  
                  Point2I point;
                  outBox.startPoint = points.size();
                  outBox.numPoints = 4;
                  
                  for (U32 i=0; i<4; i++)
                  {
                     uint16_t xp;
                     uint16_t yp;
                     stream.read(&xp);
                     stream.read(&yp);
                     points.push_back(Point2I(xp, yp));
                  }
                  
                  stream.read(&outBox.mask);
                  stream.read(&outBox.flags);
                  stream.read(&outBox.scale);
                  size -= 20;
                  
                  boxes.push_back(outBox);
               }
            }
         }
         else if (block.ident == 1279345491) // SCAL
         {
            scaleBands.clear();
            
            for (U32 i=0; i<NumScaleSlots; i++)
            {
               ScaleInfo info;
               stream.read(&info.sy1[0]);
               stream.read(&info.sy1[1]);
               stream.read(&info.sy2[0]);
               stream.read(&info.sy2[1]);
               scaleBands.push_back(info);
            }
         }
         else
         {
            return false;
         }
      }
      
      // Decode BOXM
      nextBoxHopList.clear();
      if (!boxmData.empty())
      {
         U32 matrixN = boxes.size();
         nextBoxHopList.resize(matrixN*matrixN, 255);
         
         MemStream boxStream(boxmData.size(), boxmData.data(), true, false);
         
         for (int row = 0; row < boxes.size(); row++)
         {
            uint8_t marker = 0;
            boxStream.read(&marker);
            if (marker != 0xFF)
            {
               return false;
            }
            
            // Runs continue until the next 0xFF, which begins the next row (or ends the block)
            while (boxStream.getStatus() == Stream::Ok)
            {
               uint8_t startCol = 0;
               uint8_t endCol   = 0;
               uint8_t value    = 0;
               
               boxStream.read(&startCol);
               if (startCol == 0xFF)
               {
                  boxStream.setPosition(boxStream.getPosition()-1);
                  break;
               }
               
               boxStream.read(&endCol);
               boxStream.read(&value);
               
               if (startCol >= matrixN ||
                   endCol > matrixN ||
                   startCol > endCol)
               {
                  return false;
               }
               
               for (int col = startCol; col <= endCol; col++)
               {
                  nextBoxHopList[(size_t)row * matrixN + (size_t)col] = value;
               }
            }
         }
      }
   }
};


BEGIN_SW_NS


class ImageSet;

DefineConsoleType(TypeLimbControlVector);

class ImageSet : public SimObject
{
   typedef SimObject Parent;
public:
   
   enum Flags
   {
      FLAG_TRANSPARENT = BIT(0)
   };
   
   StringTableEntry mFormatString;
   Point2I mOffset;
   std::vector<Image> mLoadedImages;
   U32 mFlags;
   
   ImageSet();
   
   ~ImageSet();
   
   void clearImages();
   
   void ensureImageLoaded(U32 n);
   
   std::string makeImageFilename(U32 n);
   static void initPersistFields();
   
   DECLARE_CONOBJECT(ImageSet);
};

class Charset : public SimObject
{
   typedef SimObject Parent;
public:
   StringTableEntry mPath;
   
public:
   DECLARE_CONOBJECT(Charset);
};

class Palette : public SimObject
{
   typedef SimObject Parent;
public:
   
   StringTableEntry mFormatString;
   Image mImageData;
   
public:
   DECLARE_CONOBJECT(Palette);
};


END_SW_NS

