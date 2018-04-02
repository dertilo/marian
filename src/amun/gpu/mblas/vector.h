#pragma once
/*
 * Vector.h
 *
 *  Created on: 8 Dec 2016
 *      Author: hieu
 */

#include "gpu/types-gpu.h"

namespace amunmt {
namespace GPU {
namespace mblas {

template<typename T>
class Vector
{
public:
  Vector()
  :size_(0)
  ,maxSize_(0)
  ,data_(nullptr)
  {
  }

  Vector(unsigned size)
  :maxSize_(0)
  ,data_(nullptr)
  {
    newSize(size);
  }

  Vector(unsigned size, const T &val)
  :maxSize_(0)
  ,data_(nullptr)
  {
    newSize(size);

    if (val) {
      abort();
    }
    else {
      HANDLE_ERROR(cudaMemsetAsync(data_, 0, size_ * sizeof(float), CudaStreamHandler::GetStream()));
    }
  }

  Vector(const std::vector<T> &vec)
  :maxSize_(0)
  {
    newSize(vec.size());
    HANDLE_ERROR( cudaMemcpyAsync(data_, vec.data(), vec.size() * sizeof(T), cudaMemcpyHostToDevice, CudaStreamHandler::GetStream()) );
  }

  Vector(const Vector<T> &other)
  :maxSize_(other.size_)
  ,size_(other.size_)
  {
    HANDLE_ERROR( cudaMalloc(&data_, size_ * sizeof(T)) );
    //std::cerr << "malloc data2:" << data_ << std::endl;
    HANDLE_ERROR( cudaMemcpyAsync(
        data_,
        other.data_,
        size_ * sizeof(T),
        cudaMemcpyDeviceToDevice,
        CudaStreamHandler::GetStream()) );
  }

  ~Vector()
  {
    //std::cerr << "~Vector=" << this << std::endl;
    HANDLE_ERROR(cudaFree(data_));
  }

  unsigned size() const
  { return size_; }

  unsigned maxSize() const
  { return maxSize_; }

  T *data()
  { return data_; }

  const T *data() const
  { return data_; }

  void resize(unsigned newSize)
  {
    if (newSize > maxSize_) {
      T *newData;
      HANDLE_ERROR( cudaMalloc(&newData, newSize * sizeof(T)) );

      if (maxSize_) {
        assert(data_);

        HANDLE_ERROR( cudaMemcpyAsync(
            newData,
            data_,
            size_ * sizeof(T),
            cudaMemcpyDeviceToDevice,
            CudaStreamHandler::GetStream()) );

        HANDLE_ERROR(cudaFree(data_));
      }
      else {
        assert(data_ == nullptr);
      }

      data_ = newData;
      maxSize_ = newSize;
    }

    size_ = newSize;
  }

  void newSize(unsigned newSize)
  {
    reserve(newSize);
    size_ = newSize;
  }

  void reserve(unsigned newSize)
  {
    if (newSize > maxSize_) {
      if (maxSize_) {
        HANDLE_ERROR(cudaFree(data_));
      }

      HANDLE_ERROR( cudaMalloc(&data_, newSize * sizeof(T)) );

      maxSize_ = newSize;
    }
  }

  void clear()
  {
    size_ = 0;
  }

  void swap(Vector &other)
  {
    std::swap(size_, other.size_);
    std::swap(maxSize_, other.maxSize_);
    std::swap(data_, other.data_);
  }

  virtual std::string Debug(unsigned verbosity = 1) const
  {
    std::stringstream strm;
    strm << size_ << " ";

    if (verbosity == 2) {
      for (unsigned i = 0; i < size_; ++i) {
        const T &ele = data_[i];
        strm << ele << " ";
      }
    }

    return strm.str();
  }

protected:
  unsigned size_, maxSize_;
  T *data_;



};

}
}
}
