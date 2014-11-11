/**
 * @file load.h
 * @author Ryan Curtin
 *
 * Load an Armadillo matrix from file.  This is necessary because Armadillo does
 * not transpose matrices on input, and it allows us to give better error
 * output.
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
#ifndef __SIM_CORE_DATA_LOAD_H
#define __SIM_CORE_DATA_LOAD_H

#include <sim/util/log.hpp>
#include <sim/core.h>
#include <string>

namespace sim {
namespace data /** Functions to load and save matrices. */ {

/**
 * Loads a matrix from file, guessing the filetype from the extension.  This
 * will transpose the matrix at load time.  If the filetype cannot be
 * determined, an error will be given.
 *
 * The supported types of files are the same as found in Armadillo:
 *
 *  - CSV (csv_ascii), denoted by .csv, or optionally .txt
 *  - ASCII (raw_ascii), denoted by .txt
 *  - Armadillo ASCII (arma_ascii), also denoted by .txt
 *  - PGM (pgm_binary), denoted by .pgm
 *  - PPM (ppm_binary), denoted by .ppm
 *  - Raw binary (raw_binary), denoted by .bin
 *  - Armadillo binary (arma_binary), denoted by .bin
 *
 * If the file extension is not one of those types, an error will be given.
 * This is preferable to Armadillo's default behavior of loading an unknown
 * filetype as raw_binary, which can have very confusing effects.
 *
 * If the parameter 'fatal' is set to true, the program will exit with an error
 * if the matrix does not load successfully.  The parameter 'transpose' controls
 * whether or not the matrix is transposed after loading.  In most cases,
 * because data is generally stored in a row-major format and SIM requires
 * column-major matrices, this should be left at its default value of 'true'.
 *
 * @param filename Name of file to load.
 * @param matrix Matrix to load contents of file into.
 * @param fatal If an error should be reported as fatal (default false).
 * @param transpose If true, transpose the matrix after loading.
 * @return Boolean value indicating success or failure of load.
 */
template<typename eT>
bool Load(const std::string& filename,
          arma::Mat<eT>& matrix,
          bool fatal = false,
          bool transpose = true);

}; // namespace data
}; // namespace sim

// Include implementation.
#include "load_impl.h"

#endif
