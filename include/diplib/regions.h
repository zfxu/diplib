/*
 * DIPlib 3.0
 * This file contains declarations for functions that work with labeled images.
 *
 * (c)2016-2019, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIP_REGIONS_H
#define DIP_REGIONS_H

#include "diplib.h"
#include "diplib/neighborlist.h"


/// \file
/// \brief Functions to label connected components and process labeled images.
/// \see regions


namespace dip {


/// \defgroup regions Labeled regions
/// \brief Label connected components and process labeled images.
///
/// Labeled images are of any unsigned integer type.
/// \{


/// \brief Labels the connected components in a binary image
///
/// The output is an unsigned integer image. Each object (respecting the connectivity,
/// see \ref connectivity) in the input image receives a unique number. This number ranges
/// from 1 to the number of objects in the image. The pixels in the output image corresponding
/// to a given object are set to this number (label). The remaining pixels in the output image
/// are set to 0.
///
/// The `minSize` and `maxSize` set limits on the size of the objects: Objects smaller than `minSize`
/// or larger than `maxSize` do not receive a label and the corresponding pixels in the output
/// image are set to zero. Setting either to zero disables the corresponding check. Setting both
/// to zero causes all objects to be labeled, irrespective of size.
///
/// The boundary conditions are generally ignored (labeling stops at the boundary). The exception
/// is `"periodic"`, which is the only one that makes sense for this algorithm.
DIP_EXPORT dip::uint Label(
      Image const& binary,
      Image& out,
      dip::uint connectivity = 0,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray const& boundaryCondition = {}
);
inline Image Label(
      Image const& binary,
      dip::uint connectivity = 0,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Label( binary, out, connectivity, minSize, maxSize, boundaryCondition );
   return out;
}

/// \brief Gets a list of object labels in the labeled image. A labeled image must be of an unsigned type.
///
/// If `background` is `"include"`, the label ID 0 will be included in the result if present in the image.
/// Otherwise, `background` is `"exclude"`, and the label ID 0 will be ignored.
DIP_EXPORT UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      String const& background = S::EXCLUDE
);
inline UnsignedArray GetObjectLabels(
      Image::View const& label,
      String const& background = S::EXCLUDE
) {
   if( label.Offsets().empty() ) {
      // This code works if either the view is regular or has a mask.
      return GetObjectLabels( label.Reference(), label.Mask(), background );
   } else {
      // When the view uses indices, we copy the data over to a new image, it's not worth while writing separate code for this case.
      return GetObjectLabels( Image( label ), {}, background );
   }
}

/// \brief Re-assigns labels to objects in a labeled image, such that all labels are consecutive.
DIP_EXPORT void Relabel( Image const& label, Image& out );
inline Image Relabel( Image const& label ) {
   Image out;
   Relabel( label, out );
   return out;
}

/// \brief Removes small objects from a labeled or binary image.
///
/// If `in` is an unsigned integer image, it is assumed to be a labeled image. The size of the objects
/// are measured using `dip::MeasurementTool`, and the labels for the objects with fewer than `threshold`
/// pixels are removed. The `connectivity` parameter is ignored.
///
/// If `in` is a binary image, `dip::Label` is called with `minSize` set to `threshold`, and the result
/// is binarized again. `connectivity` is passed to the labeling function.
///
/// The operation on a binary image is equivalent to an area opening with parameter `threshold`
/// (see `dip::AreaOpening`). The same is not true for the labeled image case, if labeled regions
/// are touching.
DIP_EXPORT void SmallObjectsRemove(
      Image const& in,
      Image& out,
      dip::uint threshold,
      dip::uint connectivity = 0
);
inline Image SmallObjectsRemove(
      Image const& in,
      dip::uint threshold,
      dip::uint connectivity = 0
) {
   Image out;
   SmallObjectsRemove( in, out, threshold, connectivity );
   return out;
}

/// \brief  Grow (dilate) labeled regions uniformly.
///
/// The regions in the labeled image `label` are dilated `iterations` steps, according to `connectivity`,
/// and optionally constrained by `mask`. If `iterations` is 0, the objects are dilated until no further
/// change is possible.
///
/// If a `mask` is given, this is the labeled equivalent to `dip::BinaryPropagation`, otherwise it works as
/// `dip::BinaryDilation` on each label. The difference between `%dip::GrowRegions` and `dip::Dilation`
/// (which can also be applied to a labeled image) is that here growing stops when different labels meet,
/// whereas in a normal dilation, the label with the larger value would grow over the one with the smaller value.
///
/// The `connectivity` parameter defines the metric, that is, the shape of the structuring element
/// (see \ref connectivity). Alternating connectivity is only implemented for 2D and 3D images.
///
/// \see dip::GrowRegionsWeighted, dip::SeededWatershed
DIP_EXPORT void GrowRegions(
      Image const& label,
      Image const& mask,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 0
);
inline Image GrowRegions(
      Image const& label,
      Image const& mask = {},
      dip::sint connectivity = -1,
      dip::uint iterations = 0
) {
   Image out;
   GrowRegions( label, mask, out, connectivity, iterations );
   return out;
}

/// \brief Grow labeled regions with a speed function given by a grey-value image.
///
/// The regions in the input image `label` are grown according to a grey-weighted distance
/// metric; the weights are given by `grey`. The optional mask image `mask` limits the
/// growing (not yet implemented!). All three images must be scalar. `label` must be of
/// an unsigned integer type, and `grey` must be real-valued.
///
/// `out` is of the type `dip::DT_LABEL`, and contains the grown regions.
///
/// Non-isotropic sampling is supported through `metric`, which assumes isotropic sampling
/// by default. See `dip::GreyWeightedDistanceTransform` for more information on how the
/// grey-weighted distance is computed.
///
/// \see dip::GrowRegions, dip::SeededWatershed
DIP_EXPORT void GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Image& out,
      Metric const& metric = { S::CHAMFER, 2 }
);
inline Image GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask = {},
      Metric const& metric = { S::CHAMFER, 2 }
) {
   Image out;
   GrowRegionsWeighted( label, grey, mask, out, metric );
   return out;
}

/// \}

} // namespace dip

#endif // DIP_REGIONS_H
