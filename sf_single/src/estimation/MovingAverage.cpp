/**
 * @file 
 * @author Denise Ratasich
 * @date 16.08.2013
 *
 * @brief Implementation of a (weighted) moving average filter (FIR
 * and IIR).
 */

#include "estimation/MovingAverage.h"

namespace estimation 
{
  MovingAverage::MovingAverage ()
  {
    // set parameters to default values
    windowSize = 3;
    isIIR = false;
    createAndInitCoefficientsWithDefaults();
  }

  MovingAverage::~MovingAverage () 
  {
    delete[] a;
    delete[] b;
  }
	
  // -----------------------------------------
  // getters and setters
  // -----------------------------------------
  unsigned int MovingAverage::getWindowSize (void)
  {
    return windowSize;
  }

  void MovingAverage::setWindowSize (unsigned int windowSize)
  {
    if (windowSize > 0  &&  this->windowSize != windowSize)
      this->windowSize = windowSize;

    // free memory of arrays with old window size
    delete[] a;
    delete[] b;

    // create arrays for coefficients and initialize
    createAndInitCoefficientsWithDefaults();
  }

  void MovingAverage::setWeightingCoefficientsIn (double *b, unsigned int size)
  {
    if (size != windowSize)
      return;		// TODO replace with throw exception
    
    // get normalization factor for weighting coefficients
    double norm = getNormalizationFactor((const double *) b, windowSize);

    // save normalized coefficients
    for (int i = 0; i < windowSize; i++)
      this->b[i] = b[i] / norm;
  }

  void MovingAverage::setWeightingCoefficientsOut (double *a, unsigned int size)
  {
    if (size != windowSize)
      return;		// TODO replace with throw exception

    // get normalization factor for weighting coefficients
    double norm = getNormalizationFactor((const double *) a, windowSize-1);

    if (norm > 0)
      isIIR = true;

    // save coefficients
    for (int i = 0; i < windowSize-1; i++)
      this->a[i] = a[i] / norm;
  }

  // -----------------------------------------
  // IEstimationMethod implementation
  // -----------------------------------------
  OutputValue MovingAverage::estimate (InputValue next)
  {
    // push the data into the queue
    in.push_front(next);
    
    // check if queue has enough elements to estimate the average with
    // the specified windowSize
    if (in.size() >= windowSize) {
      // apply filter formula
      // y[n] = sum_k a_k y[n-k] + sum_k b_k x[n-k]
      double y = b[0] * in[0].getValue();
      for (int i = 1; i < windowSize; i++) {
	// in[0] ... front/new, in[windowSize-1] ... back/old
	y += a[i-1] * out[i-1].getValue() + b[i] * in[i].getValue();
      }
      // normalization
      if (isIIR)
	y /= 2;

      // prepare output value (new estimate)
      OutputValue outY;
      outY.setValue(y);
      outY.setVariance(0);
      outY.setJitter(in.front().getJitter());
      out.push_front(outY);

      // delete oldest element for the next estimation
      in.pop_back();
      out.pop_back();
    } else {
      // set output = input for the first #window-size elements
      OutputValue nextOut;
      nextOut.setValue(next.getValue());
      nextOut.setVariance(0);
      nextOut.setJitter(next.getJitter());
      out.push_front(nextOut);
    }
    
    return out.front();
  }

  OutputValue MovingAverage::getLastEstimate(void) 
  {
    return out.front();
  }

  // -----------------------------------------
  // private functions
  // -----------------------------------------
  double MovingAverage::getNormalizationFactor(const double *coefficients, unsigned int size)
  {
    // calculate normalization factor
    double norm = 0;

    for (int i = 0; i < size; i++)
      norm += coefficients[i];

    return norm;
  }

  void MovingAverage::createAndInitCoefficientsWithDefaults(void)
  {
    this->b = new double[windowSize];
    this->a = new double[windowSize-1];

    for (int i = 0; i < windowSize-1; i++) {
      b[i] = 1.0/windowSize;		// TODO replace with hamming! normalized!
      a[i] = 0;
    }
    b[windowSize-1] = 1.0/windowSize;	// normalized!
  }
}
