/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2021, InterDigital
 * Copyright (c) 2021-2025, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the copyright holder(s) nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MM_STATISTICS_H_
#define _MM_STATISTICS_H_

#include <algorithm>  // for std::min and std::max
#include <cmath>      // for pow and sqrt,
#include <limits>     // for nan

namespace mm {

constexpr double CLIP = 99.99;

namespace Statistics {

struct Results {
  double      min;
  double      max;
  double      mean;
  double      variance;
  double      stdDev;
  long double sum;  // use long double to reduce risks of overflow
  double      minkowsky;
};

// sampler is a lambda func that takes size_t parameter and return associated
// sample value as a double (use closure to store the iterated array of values).
template <typename F>
inline void compute( size_t nbSamples, F&& sampler, Results& output, double clip = CLIP ) {
  output.min       = std::numeric_limits<double>::quiet_NaN();
  output.max       = std::numeric_limits<double>::quiet_NaN();
  output.mean      = std::numeric_limits<double>::quiet_NaN();
  output.variance  = std::numeric_limits<double>::quiet_NaN();
  output.stdDev    = std::numeric_limits<double>::quiet_NaN();
  output.sum       = std::numeric_limits<double>::quiet_NaN();
  output.minkowsky = std::numeric_limits<double>::quiet_NaN();

  if ( nbSamples == 0 ) { return; }

  double fract     = 1.0 / nbSamples;
  output.minkowsky = 0.0;

  // init mean, min and max
  const double sample = (std::min)( sampler( 0 ), clip);
  output.mean         = fract * sample;  // we do not use sum for the computation since it might be overflow
  output.max          = sample;
  output.min          = sample;
  output.sum          = sample;
  // compute mean, min and max
  for ( size_t i = 1; i < nbSamples; ++i ) {
    const double sample = (std::min)( sampler( i ), clip);
    output.mean += fract * sample;  // we do not use sum for the computation since it might be overflow
    output.max = std::max( output.max, sample );
    output.min = std::min( output.min, sample );
    output.sum += sample;
  }
  // compute variance
  output.variance = 0.0;
  for ( size_t i = 0; i < nbSamples; ++i ) { output.variance += fract * pow( sampler( i ) - output.mean, 2.0 ); }
  output.stdDev = sqrt( output.variance );

  // compute Minkowsky with parameter ms=3
  const double ms = 3;
  // finalize the code here
  for ( size_t i = 0; i < nbSamples; i++ ) { output.minkowsky += fract * pow( abs( sampler( i ) ), ms ); }
  output.minkowsky = pow( output.minkowsky, 1.0 / ms );
}

//
inline void printToLog( Results& stats, std::string prefix, std::ostream& out ) {
  out << prefix << "Min=" << stats.min << std::endl;
  out << prefix << "Max=" << stats.max << std::endl;
  out << prefix << "Sum=" << stats.sum << std::endl;
  out << prefix << "Mean=" << stats.mean << std::endl;
  out << prefix << "Variance=" << stats.variance << std::endl;
  out << prefix << "StdDev=" << stats.stdDev << std::endl;
  out << prefix << "Minkowsky=" << stats.minkowsky << std::endl;
}

};  // namespace Statistics

}  // namespace mm

#endif
