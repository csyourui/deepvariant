/*
 * Copyright 2024 Google LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#if true  // Trick to stop tooling from moving the #include around.
// MUST appear before any standard headers are included.
#include <pybind11/pybind11.h>
#endif

#include <pybind11/stl.h>

#include "third_party/nucleus/core/python/type_caster_nucleus_status.h"
#include "third_party/nucleus/core/python/type_caster_nucleus_statusor.h"
#include "third_party/nucleus/io/bed_reader.h"
#include "third_party/nucleus/protos/range.pb.h"
#include "third_party/nucleus/util/python/type_caster_nucleus_proto_ptr.h"
#include "pybind11_protobuf/native_proto_caster.h"

namespace py = pybind11;

PYBIND11_MODULE(bed_reader, m) {
  pybind11_protobuf::ImportNativeProtoCasters();
  using namespace ::nucleus;  // NOLINT

  py::classh<BedReader>(m, "BedReader")
      .def_static("from_file", &BedReader::FromFile, py::arg("bedPath"),
                  py::arg("options"))
      .def("iterate",
           [](const BedReader& self) {
             auto cpp_result = self.Iterate();
             auto ret0 = py::cast(std::move(cpp_result));
             auto postproc = py::module_::import(
                 "third_party.nucleus.io.clif_postproc");
             return postproc.attr("WrappedBedIterable")(ret0);
           })
      .def("query",
          [](BedReader& self, nucleus::genomics::v1::Range range) {
            auto cpp_result = self.Query(range);
            auto ret0 = py::cast(std::move(cpp_result));
            auto postproc = py::module_::import(
                "third_party.nucleus.io.clif_postproc");
            return postproc.attr("WrappedBedIterable")(ret0);
          },
          py::arg("region"))
      .def("has_index", &BedReader::HasIndex)
      .def("__enter__", [](py::object self) { return self; })
      .def("__exit__", [](BedReader& self, py::args) { return self.Close(); })
      .def_property_readonly("header", &BedReader::Header);
  py::classh<BedIterable>(m, "BedIterable")
      .def("PythonNext", &BedIterable::PythonNext, py::arg("bed"))
      .def("Release", &BedIterable::Release)
      .def("__enter__", [](py::object self) { return self; })
      .def("__exit__", [](BedIterable& self, py::args) { return self.PythonExit(); });
}
