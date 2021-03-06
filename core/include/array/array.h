/**
 * @file   array.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2016 MIT and Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @section DESCRIPTION
 *
 * This file defines class Array. 
 */

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "array_read_state.h"
#include "array_schema.h"
#include "constants.h"
#include "fragment.h"




/* ********************************* */
/*             CONSTANTS             */
/* ********************************* */

/**@{*/
/** Return code. */
#define TILEDB_AR_OK          0
#define TILEDB_AR_ERR        -1
/**@}*/

class ArrayReadState;
class Fragment;




/** Manages a TileDB array object. */
class Array {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */
  
  /** Constructor. */
  Array();

  /** Destructor. */
  ~Array();




  /* ********************************* */
  /*             ACCESSORS             */
  /* ********************************* */

  /** Returns the array schema. */
  const ArraySchema* array_schema() const;

  /** Returns the ids of the attributes the array focuses on. */
  const std::vector<int>& attribute_ids() const;

  /** Returns the number of fragments in this array. */
  int fragment_num() const;

  /** Returns the fragment objects of this array. */
  std::vector<Fragment*> fragments() const;

  /** Returns the array mode. */
  int mode() const;

  /**
   * Checks if an attribute buffer has overflown during a read operation.
   *
   * @param attribute_id The id of the attribute that is being checked.
   * @return *true* if the attribute buffer has overflown and *false* otherwise.
   */
  bool overflow(int attribute_id) const;

  /**
   * Performs a read operation in an array, which must be initialized with mode
   * TILEDB_ARRAY_READ. The function retrieves the result cells that lie inside
   * the subarray specified in init() or reset_subarray(). The results are
   * written in input buffers provided by the user, which are also allocated by
   * the user. Note that the results are written in the buffers in the same
   * order they appear on the disk, which leads to maximum performance. 
   * 
   * @param buffers An array of buffers, one for each attribute. These must be
   *     provided in the same order as the attributes specified in
   *     init() or reset_attributes(). The case of variable-sized attributes is
   *     special. Instead of providing a single buffer for such an attribute,
   *     **two** must be provided: the second will hold the variable-sized cell
   *     values, whereas the first holds the start offsets of each cell in the
   *     second buffer.
   * @param buffer_sizes The sizes (in bytes) allocated by the user for the
   *     input buffers (there is a one-to-one correspondence). The function will
   *     attempt to write as many results as can fit in the buffers, and
   *     potentially alter the buffer size to indicate the size of the *useful*
   *     data written in the buffer. If a buffer cannot hold all results, the
   *     function will still succeed, writing as much data as it can and turning
   *     on an overflow flag which can be checked with function overflow(). The
   *     next invocation will resume for the point the previous one stopped,
   *     without inflicting a considerable performance penalty due to overflow.
   * @return TILEDB_AR_OK for success and TILEDB_AR_ERR for error.
   */
  int read(void** buffers, size_t* buffer_sizes); 

  /** Returns the subarray in which the array is constrained. */
  const void* subarray() const;




  /* ********************************* */
  /*              MUTATORS             */
  /* ********************************* */

  /**
   * Consolidates all fragments into a new single one, on a per-attribute basis.
   *
   * @return TILEDB_AR_OK for success and TILEDB_AR_ERR for error.
   */
  int consolidate();

  /**
   * Consolidates all fragment into a new single one, focusing on a specific
   * attribute.
   *
   * @param new_fragment The new consolidated fragment object.
   * @param attribute_i The id of the target attribute.
   */
  int consolidate(
      Fragment* new_fragment,
      int attribute_id);

  /**
   * Finalizes the array, properly freeing up memory space.
   *
   * @return TILEDB_AR_OK on success, and TILEDB_AR_ERR on error.
   */
  int finalize();

  /**
   * Initializes a TileDB array object.
   *
   * @param array_schema The array schema.
   * @param mode The mode of the array. It must be one of the following:
   *    - TILEDB_ARRAY_WRITE 
   *    - TILEDB_ARRAY_WRITE_UNSORTED 
   *    - TILEDB_ARRAY_READ 
   * @param subarray The subarray in which the array read/write will be
   *     constrained on. If it is NULL, then the subarray is set to the entire
   *     array domain. For the case of writes, this is meaningful only for
   *     dense arrays, and specifically dense writes.
   * @param attributes A subset of the array attributes the read/write will be
   *     constrained on. A NULL value indicates **all** attributes (including
   *     the coordinates in the case of sparse arrays).
   * @param attribute_num The number of the input attributes. If *attributes* is
   *     NULL, then this should be set to 0.
   * @return TILEDB_AR_OK on success, and TILEDB_AR_ERR on error.
   */
  int init(
      const ArraySchema* array_schema, 
      int mode,
      const char** attributes,
      int attribute_num,
      const void* range);

  /**
   * Resets the attributes used upon initialization of the array. 
   *
   * @param attributes The new attributes to focus on. If it is NULL, then
   *     all the attributes are used (including the coordinates in the case of
   *     sparse arrays).
   * @param attribute_num The number of the attributes. If *attributes* is NULL,
   *     then this should be 0.
   * @return TILEDB_AR_OK on success, and TILEDB_AR_ERR on error.
   */
  int reset_attributes(const char** attributes, int attribute_num);

  /**
   * Resets the subarray used upon initialization of the array. This is useful
   * when the array is used for reading, and the user wishes to change the
   * query subarray without having to finalize and re-initialize the array
   * with a different subarray.
   *
   * @param subarray The new subarray. Note that the type of the values in
   *     *subarray* should match the coordinates type in the array schema.
   * @return TILEDB_AR_OK on success, and TILEDB_AR_ERR on error.
   */
  int reset_subarray(const void* subarray);

  /**
   * Performs a write operation in the array. The cell values are provided
   * in a set of buffers (one per attribute specified upon initialization).
   * Note that there must be a one-to-one correspondance between the cell
   * values across the attribute buffers.
   *
   * The array must be initialized in one of the following write modes,
   * each of which having a different behaviour:
   *    - TILEDB_ARRAY_WRITE: \n
   *      In this mode, the cell values are provided in the buffers respecting
   *      the cell order on the disk. It is practically an **append** operation,
   *      where the provided cell values are simply written at the end of
   *      their corresponding attribute files. This mode leads to the best
   *      performance. The user may invoke this function an arbitrary number
   *      of times, and all the writes will occur in the same fragment. 
   *      Moreover, the buffers need not be synchronized, i.e., some buffers
   *      may have more cells than others when the function is invoked.
   *    - TILEDB_ARRAY_WRITE_UNSORTED: \n
   *      This mode is applicable to sparse arrays, or when writing sparse
   *      updates to a dense array. One of the buffers holds the coordinates.
   *      The cells in this mode are given in an arbitrary, unsorted order
   *      (i.e., without respecting how the cells must be stored on the disk
   *      according to the array schema definition). Each invocation of this
   *      function internally sorts the cells and writes them to the disk on the
   *      proper order. In addition, each invocation creates a **new** fragment.
   *      Finally, the buffers in each invocation must be synced, i.e., they
   *      must have the same number of cell values across all attributes.
   * 
   * @param buffers An array of buffers, one for each attribute. These must be
   *     provided in the same order as the attributes specified in
   *     init() or reset_attributes(). The case of variable-sized attributes is
   *     special. Instead of providing a single buffer for such an attribute,
   *     **two** must be provided: the second holds the variable-sized cell
   *     values, whereas the first holds the start offsets of each cell in the
   *     second buffer.
   * @param buffer_sizes The sizes (in bytes) of the input buffers (there is
   *     a one-to-one correspondence).
   * @return TILEDB_AR_OK for success and TILEDB_AR_ERR for error.
   */
  int write(const void** buffers, const size_t* buffer_sizes); 

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** The array schema. */
  const ArraySchema* array_schema_;
  /** The read state of the array. */
  ArrayReadState* array_read_state_;
  /** 
   * The ids of the attributes the array is initialized with. Note that the
   * array may be initialized with a subset of attributes when writing or
   * reading.
   */
  std::vector<int> attribute_ids_;
  /** The array fragments. */
  std::vector<Fragment*> fragments_;
  /** 
   * The array mode. It must be one of the following:
   *    - TILEDB_WRITE 
   *    - TILEDB_WRITE_UNSORTED 
   *    - TILEDB_READ 
   */
  int mode_;
  /**
   * The subarray in which the array is constrained. Note that the type of the
   * range must be the same as the type of the array coordinates.
   */
  void* subarray_;




  /* ********************************* */
  /*           PRIVATE METHODS         */
  /* ********************************* */
  
  /** 
   * Returns a new fragment name, which is in the form: <br>
   * .__<process_id>_<timestamp>
   *
   * Note that this is a temporary name, initiated by a new write process.
   * After the new fragmemt is finalized, the array will change its name
   * by removing the leading '.' character. 
   *
   * @return A new special fragment name.
   */
  std::string new_fragment_name() const;

  /**
   * Opens the existing fragments in TILEDB_ARRAY_READ_MODE.
   *
   * @return TILEDB_AR_OK for success and TILEDB_AR_ERR for error.
   */
  int open_fragments();

  /** 
   * Appropriately sorts the fragment names based on their name timestamps.
   */
  void sort_fragment_names(std::vector<std::string>& fragment_names) const;
};

#endif
