#!/usr/local/bin/python3

# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from read_values import (
    call_reader_function,
    read_byte_value,
    read_int_value,
    read_string_value,
    read_long_value,
)
from numeric_conversion import int_to_hex_string


def read_object_header(message_bytes, offset):
    object_base = {}
    (object_base["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (object_base["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    return object_base, offset


def read_object_part(message_bytes, offset):
    object_part, offset = read_object_header(message_bytes, offset)

    offset += 2 * object_part["Size"]
    return object_part, offset


def read_int_part(message_bytes, offset):
    int_part, offset = read_object_header(message_bytes, offset)

    int_value, offset = call_reader_function(message_bytes, offset, read_int_value)
    int_part["Value"] = int_to_hex_string(int_value)
    return int_part, offset


def read_region_part(message_bytes, offset):
    region_part = {}
    (region_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (region_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (region_part["Name"], offset) = read_string_value(
        message_bytes, region_part["Size"], offset
    )
    return (region_part, offset)


def read_event_id_part(message_bytes, offset):
    event_id_part = {}
    (event_id_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (event_id_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["LongCode1"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["EventIdThread"], offset) = call_reader_function(
        message_bytes, offset, read_long_value
    )
    (event_id_part["LongCode2"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    (event_id_part["EventIdSequence"], offset) = call_reader_function(
        message_bytes, offset, read_long_value
    )
    return (event_id_part, offset)


def read_raw_boolean_part(message_bytes, offset):
    bool_part = {}
    (bool_part["Size"], offset) = call_reader_function(
        message_bytes, offset, read_int_value
    )
    (bool_part["IsObject"], offset) = call_reader_function(
        message_bytes, offset, read_byte_value
    )
    bool_val = 0
    (bool_val, offset) = call_reader_function(message_bytes, offset, read_byte_value)
    bool_part["Value"] = "False" if bool_val == 0 else "True"
    return (bool_part, offset)
