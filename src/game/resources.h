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
   
   static bool PointInConvexPoly(Point2I* poly, U32 n, Point2I p)
   {
       if (n < 3) return false;

       if (PointOnEdge(poly, n, p)) return true;

       int sign = 0;
       for (int i = 0; i < n; i++) {
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
   
   uint8_t getNextBoxIndex(int src, int dst)
   {
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

