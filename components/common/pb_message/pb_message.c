#include "pb_message.h"

#include <stdlib.h>

#include "components/common/envelope.pb.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "result.h"

result_t pb_message_encode(const void *src_struct, const pb_msgdesc_t *fields,
                           uint8_t **out_data, size_t *out_length) {
  bool status;

  if (out_data == NULL || out_length == NULL) {
    return RESULT_FAIL;
  }

  *out_data = NULL;
  *out_length = 0;

#if defined(PB_PROTO_HEADER_VERSION) && PB_PROTO_HEADER_VERSION >= 40
  size_t size = 0;
  const pb_msgdesc_t *msgdesc = (const pb_msgdesc_t *)fields;
  status = pb_get_encoded_size(&size, msgdesc, src_struct);
  *out_length = size;

  if (!status) {
    return RESULT_ERR_BAD_FORMAT;
  }

  *out_data = (uint8_t *)malloc(*out_length);
  if (*out_data == NULL) {
    *out_length = 0;
    return RESULT_ERR_NO_MEM;
  }

  pb_ostream_t ostream = pb_ostream_from_buffer(*out_data, *out_length);
  status = pb_encode(&ostream, msgdesc, src_struct);
#else
  status = pb_get_encoded_size(out_length, fields, src_struct);

  if (!status) {
    return RESULT_ERR_BAD_FORMAT;
  }

  *out_data = (uint8_t *)malloc(*out_length);
  if (*out_data == NULL) {
    *out_length = 0;
    return RESULT_ERR_NO_MEM;
  }

  pb_ostream_t ostream = pb_ostream_from_buffer(*out_data, *out_length);
  status = pb_encode(&ostream, fields, src_struct);
#endif

  if (!status) {
    free(*out_data);
    *out_data = NULL;
    *out_length = 0;
    return RESULT_FAIL;
  }

  return RESULT_OK;
}

result_t pb_message_decode(const uint8_t *byte_buffer, size_t size,
                           const pb_msgdesc_t *fields, size_t struct_size,
                           void **out_struct) {
  if (out_struct == NULL) {
    return RESULT_FAIL;
  }

  *out_struct = NULL;

  void *buf = malloc(struct_size);
  if (buf == NULL) {
    return RESULT_ERR_NO_MEM;
  }

  memset(buf, 0, struct_size);

  pb_istream_t istream = pb_istream_from_buffer(byte_buffer, size);
  bool status;

#if defined(PB_PROTO_HEADER_VERSION) && PB_PROTO_HEADER_VERSION >= 40
  const pb_msgdesc_t *msgdesc = (const pb_msgdesc_t *)fields;
  status = pb_decode(&istream, msgdesc, buf);
#else
  status = pb_decode(&istream, fields, buf);
#endif

  if (!status) {
    free(buf);
    return RESULT_FAIL;
  }

  *out_struct = buf;
  return RESULT_OK;
}

result_t pb_message_decode_into(const uint8_t *byte_buffer, size_t size,
                                const pb_msgdesc_t *fields, size_t struct_size,
                                void *out_struct) {
  if (byte_buffer == NULL || fields == NULL || out_struct == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  memset(out_struct, 0, struct_size);

  pb_istream_t istream = pb_istream_from_buffer(byte_buffer, size);
  bool status;

#if defined(PB_PROTO_HEADER_VERSION) && PB_PROTO_HEADER_VERSION >= 40
  const pb_msgdesc_t *msgdesc = (const pb_msgdesc_t *)fields;
  status = pb_decode(&istream, msgdesc, out_struct);
#else
  status = pb_decode(&istream, fields, out_struct);
#endif

  if (!status) {
    return RESULT_FAIL;
  }

  return RESULT_OK;
}

result_t pb_message_decode_envelope(const uint8_t *byte_buffer, size_t size,
                                    uint8_t **out_data) {
  // cursed memory reallocation to not have to alloc dynamically

  return RESULT_ERR_NO_MEM;
}
