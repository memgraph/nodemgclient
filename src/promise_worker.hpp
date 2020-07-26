// Copyright (c) 2016-2020 Memgraph Ltd. [https://memgraph.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <napi.h>

// Based on
// https://github.com/nodejs/node-addon-api/issues/231#issuecomment-528986145.

class PromiseWorker : public Napi::AsyncWorker {
 public:
  PromiseWorker(Napi::Promise::Deferred const &d, const char *resource_name)
      : AsyncWorker(get_fake_callback(d.Env()).Value(), resource_name),
        deferred(d) {}
  PromiseWorker(Napi::Promise::Deferred const &d)
      : AsyncWorker(get_fake_callback(d.Env()).Value()), deferred(d) {}

  virtual void Resolve(Napi::Promise::Deferred const &deferred) = 0;

  void OnOK() override { Resolve(deferred); }

  void OnError(Napi::Error const &error) override {
    deferred.Reject(error.Value());
  }

 private:
  static Napi::Value noop(Napi::CallbackInfo const &info) {
    return info.Env().Undefined();
  }

  Napi::Reference<Napi::Function> const &get_fake_callback(
      Napi::Env const &env) {
    static Napi::Reference<Napi::Function> fake_callback =
        Napi::Reference<Napi::Function>::New(Napi::Function::New(env, noop), 1);
    fake_callback.SuppressDestruct();

    return fake_callback;
  }

  Napi::Promise::Deferred deferred;
};
