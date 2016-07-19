/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"

namespace dip {
namespace Framework {

// Part of the next two functions
static inline void SingletonExpandedSize(
      UnsignedArray& size,
      const UnsignedArray& size2
) {
   if( size.size() < size2.size() ) {
      size.resize( size2.size(), 1 );
   }
   for( dip::uint jj = 0; jj < size2.size(); ++jj ) {
      if( size[jj] != size2[jj] ) {
         if( size[jj] == 1 ) {
            size[jj] = size2[jj];
         } else if( size2[jj] != 1 ) {
            dip_Throw( E::DIMENSIONS_DONT_MATCH );
         }
      }
   }
}

// Figure out what the size of the images must be.
UnsignedArray SingletonExpandedSize(
      const ImageRefArray& in
) {
   UnsignedArray size = in[0].get().Dimensions();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      UnsignedArray size2 = in[ii].get().Dimensions();
      SingletonExpandedSize( size, size2 );
   }
   return size;
}

// Idem as above.
UnsignedArray SingletonExpandedSize(
      const ImageArray& in
) {
   UnsignedArray size = in[0].Dimensions();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      UnsignedArray size2 = in[ii].Dimensions();
      SingletonExpandedSize( size, size2 );
   }
   return size;
}

// Adjust the size of one image.
void SingletonExpansion(
      Image& in,
      const UnsignedArray& size
) {
   dip::uint ndims = size.size();
   if( in.Dimensionality() < ndims ) {
      in.ExpandDimensionality( ndims );
   }
   UnsignedArray size2 = in.Dimensions();
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      if( size2[ii] != size[ii] ) {
         in.ExpandSingletonDimension( ii, size[ii] );
      }
   }
}

// Find best processing dimension, which is the one with the smallest stride,
// except if that dimension is very small and there's a longer dimension.
dip::uint OptimalProcessingDim(
      const Image& in
) {
   constexpr dip::uint SMALL_IMAGE = 63;  // A good value would depend on the size of cache.
   const IntegerArray& strides = in.RefStrides();
   const UnsignedArray& dims = in.RefDimensions();
   dip::uint processingDim = 0;
   for( dip::uint ii = 1; ii < strides.size(); ++ii ) {
      if( strides[ii] < strides[processingDim] ) {
         if(( dims[ii] > SMALL_IMAGE ) || ( dims[ii] > dims[processingDim] )) {
            processingDim = ii;
         }
      } else if(( dims[processingDim] <= SMALL_IMAGE ) && ( dims[ii] > dims[processingDim] )) {
         processingDim = ii;
      }
   }
   return processingDim;
}

} // namespace Framework
} // namespace dip