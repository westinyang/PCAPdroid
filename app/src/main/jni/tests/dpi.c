/*
 * This file is part of PCAPdroid.
 *
 * PCAPdroid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PCAPdroid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PCAPdroid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2022 - Emanuele Faranda
 */

#include "test_utils.h"

/* ******************************************************* */

// Called on send_connections_dump. pd->new_conns contains the dumped
// connections. Ensures that metadata is correctly extracted from
// network traffic.
static void extract_metadata_cb(pcapdroid_t *pd) {
  conn_and_tuple_t *conn;

  // DNS request without reply
  conn = assert_conn(pd, IPPROTO_UDP, "8.8.8.8", 53, "example.org");
  assert(conn->tuple.src_port == htons(48037));

  // DNS (TCP)
  assert_conn(pd, IPPROTO_TCP, "8.8.8.8", 53, "f-droid.org");

  // Guess host name from previous DNS request
  assert_conn(pd, IPPROTO_TCP, "149.202.95.241", 80, "f-droid.org");

  // HTTP
  conn = assert_conn(pd, IPPROTO_TCP, "216.58.208.164", 80, "www.google.com");
  assert(!strcmp(conn->data->url, "www.google.com/imghp?test=1&v2=2"));
  conn = assert_conn(pd, IPPROTO_TCP, "385d:1ee:e3c9:9c5f::2004", 80, "www.google.com");
  assert(!strcmp(conn->data->url, "www.google.com/imghp?test=1&v2=2"));

  // TLS
  conn = assert_conn(pd, IPPROTO_TCP, "142.250.180.131", 443, "google.it");
  assert(conn->data->l7proto == NDPI_PROTOCOL_TLS);
  conn = assert_conn(pd, IPPROTO_TCP, "2ed5:9050:81e9:4b68:248:1893:25c8:1946", 443, "example.org");
  assert(conn->data->l7proto == NDPI_PROTOCOL_TLS);
}

static void test_metadata_extraction() {
  conn_and_tuple_t *conn;
  pcapdroid_t *pd = pd_init(PCAP_PATH "/metadata.pcap");

  pd->cb.send_connections_dump = extract_metadata_cb;
  pd_run(pd);

  pd_free(pd);
}

/* ******************************************************* */

int main(int argc, char **argv) {
  add_test("extract", test_metadata_extraction);

  run_test(argc, argv);
  return 0;
}
