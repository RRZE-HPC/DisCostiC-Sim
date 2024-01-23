
#ifndef DataType_HPP
#define DataType_HPP

#include <set>
#include <vector>

#include "enum.hpp"				//!<  enumerated types

namespace DataType{
    
    /**
     * \brief a class to represent fixed-size three-dimensional vector data-type of arbitrary types with coefficients type, addr, size
     */
    template <typename Tx, typename Ty, typename Tz>
    class vector3T {
    public:
        ///public variables type, addr, size in three-dimensional of integer and btime_t datatype
        Tx type; //!< type, which can be IN=1 or OUT=2 indicated by a '<' or '>' in the schedule
        Ty addr; //!< actual address in memory where to read/write
        Tz size; //!< size of data to read/write (referenced by the address addr) in bytes
        
        /// constructors
        vector3T() : type(0), addr(0), size(0) {};
        
        vector3T(const vector3T &elem) : type(elem.type), addr(elem.addr), size(elem.size) {};
        
        vector3T(Tx t, Ty a, Tz s) : type(t), addr(a), size(s) {};
        
        /**
         * \brief a operator=() member
         * \param o of type scalarT.
         */
        vector3T& operator=(const vector3T &elem) {
            type = elem.type;
            addr = elem.addr;
            size = elem.size;
            return *this;
        };
    };
}

using real_t = double;
using size_t = std::size_t;

using DisCosTiC_Timetype = double;//unsigned long int;
using DisCosTiC_Datatype = std::size_t;
using DisCosTiC_Indextype = std::size_t;
using idSetT = std::set<DisCosTiC_Indextype>;
//using idMapT = std::map<unsigned long,DisCosTiC_Indextype>;


using vec1T = std::vector<DisCosTiC_Indextype>;
using Time = std::vector<DisCosTiC_Timetype>;
using Real = std::vector<real_t>;
using Timevec2T = std::vector<std::vector<DisCosTiC_Timetype>>;
using vec3T = DataType::vector3T<size_t, size_t, DisCosTiC_Timetype>;
using locop_t = std::vector<std::pair<DisCosTiC_Indextype, DisCosTiC::Mode_t> >; //!< identify local operations for dependencies
using locopPair_t = std::pair<locop_t, locop_t>;

namespace DisCosTiC{
  /** \brief a time stepping loop */
template <typename scalarT>
struct std_iter : std::iterator<std::input_iterator_tag, scalarT>{
  std_iter(scalarT it) : it(it) { }

  scalarT operator *() const { return it; }

  scalarT const* operator ->() const { return &it; }

  std_iter& operator ++() {
    ++it;
    return *this;
  }

  std_iter operator ++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator ==(std_iter const& other) const {
    return it == other.it;
  }

  bool operator !=(std_iter const& other) const {
    return not (*this == other);
  }

protected:
    scalarT it;
};

/**
*\brief iterator ranges for each entityTypes to support iteration with range-based for loops. Iterating over sets of entityTypes is one of the most common operation. Our infrastructure implements this custom range-based for loops in the C++ ways by providing iterators and matching begin(), end() and stepSize(scalarT stepSize) methods.
*/
template <typename scalarT>
struct iteratorRange {
   /// wrapper class of range-based for loop with certain step size
  struct iteratorRangeStep {
    struct iter : std_iter<scalarT> {
      iter(scalarT it, scalarT stepSize): std_iter<scalarT>(it), stepSize(stepSize) { }

      iter& operator ++() {
    std_iter<scalarT>::it += stepSize;
        return *this;
      }

      iter operator ++(int) {
    auto tmp = *this;
    ++*this;
        return tmp;
      }

      bool operator ==(iter const& other) const {
    return stepSize > 0 ? std_iter<scalarT>::it >= other.it: std_iter<scalarT>::it < other.it;
      }

      bool operator !=(iter const& other) const {
    return not (*this == other);
      }

      protected:
      scalarT stepSize;
    };
    
    iteratorRangeStep(scalarT begin, scalarT end, scalarT stepSize): begin_(begin, stepSize), end_(end, stepSize) { }

    iter begin() const { return begin_; }

    iter end() const { return end_; }

    private:
      iter begin_;
      iter end_;
  }; // end of iteratorRangeStep struct
  
  iteratorRangeStep stepSize(scalarT stepSize) {    return {*begin_, *end_, stepSize}; } //!< Constructs an iterator range on [begin,end) with certain stepSize
  
  struct iter : std_iter<scalarT>{
    iter(scalarT it) : std_iter<scalarT>(it) { }
  };

  iteratorRange(scalarT begin, scalarT end) : begin_(begin), end_(end) { } //!< Constructs an iterator range on [begin,end)

  iter begin() const{ return begin_; }  //!< returns an iterator pointing to the begin of the range

  iter end() const { return end_; } //!< returns an iterator pointing past the end of the range

  private:
    iter begin_;
    iter end_;
}; // end of iteratorRange struct

template <typename scalarT>
iteratorRange<scalarT> getRange(scalarT begin, scalarT end) {
  return {begin, end};
}

template <typename scalarT>
iteratorRange<scalarT> getRange(scalarT end) {
  return {0, end};
}
} //namespace DisCosTiC

#endif //!< end DataType_HPP
