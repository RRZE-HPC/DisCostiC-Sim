
#ifndef ENUM_HPP
#define ENUM_HPP

namespace DisCosTiC {
/*!
 *  \addtogroup DisCosTiC
    *  @{
     */
    ///   The Operation_t enum defines different opertation types of entities.
    enum Operation_t{
        SEND = 1,         	//!< send operation type
        RECV = 2,        	//!< recv operation type
        COMP = 3,        	//!< compuation operation type
        MSG =4,         	//!< message / transmission operation type 
    };
    /*! @} */

    /*!
     *  \addtogroup DisCosTiC
     *  @{
     */
    /// The Mode_t enum defines operation type of SEND and RECV entities (i.e., either blocking calls that reutn on;y on completion of operation or non-blocking calls that return with start of operation)
    enum Mode_t {
        NONBLOCKING, //!< isend and irecv MPI routines, non-blocking: next operation not be executed before starting of previous operation
        BLOCKING,     //!< send and recv MPI routines, blocking: next operation not be executed before finishing of previous operation
    };
    /*! @} */
    
} // end namespace DisCosTiC


#endif //!< end ENUM_HPP

