#ifndef PB_MESSAGE_H
#define PB_MESSAGE_H

#include "pb.h"
#include "result.h"
#include "stdint.h"


/**
 * @brief The result of a nanopb decoding,the resulting void* message is
 * allocated and populated with the appropriate object
 * @param result result_t type, should be RESULT_OK if the decoding was
 * succesful
 * @param message void pointer to the decoded message struct
 */
typedef struct {
  result_t result;
  void *message;
} pb_message_t;

/**
 * @brief The result of a nanopb encoding
 * @param result result_t type, should eb RESULT_OK if the encoding was
 * succesful
 * @param buffer bytearray of the resulting encoding
 * @param buffer_length size of buffer
 */
typedef struct {
  result_t result;
  uint8_t *data;
  size_t length;
} pb_encoding_t;

/**
 * @brief Encodes any nanopb message struct into a heap allocated buffer
 *
 * This function performs a "dry run" of the encoding to calculate the exact
 * buffer size needed, allocates memory, and then performs the actual encoding.
 *
 * @param src_struct Pointer to the message struct to be encoded (e.g.,
 * &my_message).
 * @param fields The message's _fields array (e.g. MyMessage_fields).
 * @return A pb_encoding_t struct, on success the result fields will be
 * RESULT_OK and the data field will be a heap allocated byte array. On failure,
 * the data field is NULL.
 */
pb_encoding_t pb_message_encode(const void *src_struct,
                                              const pb_field_t fields[]);

/**
 * @brief Decodes any nanopb message struct into a heap allocated struct
 *
 * @param buffer Pointer to the byte array of the serialized message
 * @param size Size of the provided buffer
 * @param fields The message's _fields array (e.g., MyMessage_fields)
 * @param struct_size the size of the target struct (e.g., sizeof(MyMessage))
 */
pb_message_t pb_message_decode(const uint8_t*buffer, size_t size, const pb_field_t fields[], size_t struct_size);
#endif
