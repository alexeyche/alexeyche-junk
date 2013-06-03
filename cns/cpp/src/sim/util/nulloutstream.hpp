/**
 * @file nulloutstream.hpp
 * @author Ryan Curtin
 * @author Matthew Amidon
 *
 * Definition of the NullOutStream class.
 *
 * This file is part of SIM 1.0.3.
 *
 * SIM is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * SIM is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details (LICENSE.txt).
 *
 * You should have received a copy of the GNU General Public License along with
 * SIM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SIM_CORE_IO_NULLOUTSTREAM_HPP
#define __SIM_CORE_IO_NULLOUTSTREAM_HPP

#include <iostream>
#include <streambuf>
#include <string>

namespace sim {
namespace io {

/**
 * Used for Log::Debug when not compiled with debugging symbols.  This class
 * does nothing and should be optimized out entirely by the compiler.
 */
class NullOutStream
{
 public:
  /**
   * Does nothing.
   */
  NullOutStream() { }

  /**
   * Does nothing.
   */
  NullOutStream(const NullOutStream& /* other */) { }

  /*
   We use (void) paramName in order to avoid the warning generated by
   -Wextra. For some currently unknown reason, simply deleting the 
   parameter name (aka, outperator<<(bool) {...}) causes a compilation
   error (with -Werror off) for only this class.
   */

  //! Does nothing.
  NullOutStream& operator<<(bool val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(short val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(unsigned short val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(int val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(unsigned int val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(long val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(unsigned long val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(float val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(double val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(long double val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(void* val) { (void) val; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(const char* str) { (void) str; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(std::string& str) { (void) str; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(std::streambuf* sb) { (void) sb; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(std::ostream& (*pf) (std::ostream&))
  { (void) pf; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(std::ios& (*pf) (std::ios&)) { (void) pf; return *this; }
  //! Does nothing.
  NullOutStream& operator<<(std::ios_base& (*pf) (std::ios_base&))
  { (void) pf; return *this; }

  //! Does nothing.
  template<typename T>
  NullOutStream& operator<<(T s)
  { (void) s; return *this; }
};

} // namespace io
} // namespace sim

#endif
