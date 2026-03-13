#ifndef PB_MESSAGE_H
#define PB_MESSAGE_H

#include "pb.h"
#include "result.h"
#include "stdint.h"
#include "components/common/message_types.pb.h"


/**
 * @brief Encode a nanopb message into a newly allocated byte buffer.
 *
 * This function first computes the encoded size, then allocates an output
 * buffer and encodes the message into it.
 *
 * @param[in]  src_struct   Pointer to the populated nanopb message struct.
 * @param[in]  fields       Nanopb field descriptor array for the message type.
 * @param[out] out_data     On success, receives a pointer to a heap-allocated
 *                          buffer containing the encoded message.
 * @param[out] out_length   On success, receives the number of valid bytes in
 *                          @p out_data.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_BAD_FORMAT if size computation fails.
 * @return RESULT_ERR_NO_MEM if allocation fails.
 * @return RESULT_FAIL if encoding fails.
 *
 * @note The caller owns @p out_data and must free() it when done.
 */
result_t pb_message_encode(const void *src_struct, const pb_field_t fields[],
                           uint8_t **out_data, size_t *out_length);

/**
 * @brief Decode a nanopb message from a byte buffer into a newly allocated
 * struct.
 *
 * This function allocates a zeroed struct buffer of @p struct_size bytes and
 * decodes the nanopb message from @p byte_buffer into it.
 *
 * @param[in]  byte_buffer   Pointer to input buffer containing the encoded
 * message.
 * @param[in]  size          Size of @p byte_buffer in bytes.
 * @param[in]  fields        Nanopb field descriptor array for the message type.
 * @param[in]  struct_size   Size (in bytes) of the destination message struct
 * type.
 * @param[out] out_struct    On success, receives a pointer to a heap-allocated
 *                           decoded message struct.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_NO_MEM if allocation fails.
 * @return RESULT_FAIL if decoding fails.
 *
 * @note The caller owns @p out_struct and must free() it when done.
 */
result_t pb_message_decode(const uint8_t *byte_buffer, size_t size,
                           const pb_field_t *fields, size_t struct_size,
                           void **out_struct);


result_t pb_message_decode_into(const uint8_t *byte_buffer, size_t size,
                                const pb_field_t *fields, size_t struct_size,
                                void *out_struct);
#endif
