#!/bin/bash

# Generates TypeScript declarations.

set -Eeuo pipefail
script_dir="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd)"

cd "$script_dir"
npx tsc index.js --declaration --allowJs --checkJs --resolveJsonModule --emitDeclarationOnly
types=$(cat index.d.ts)
cat << EOF > index.d.ts
// Copyright (c) 2016-2021 Memgraph Ltd. [https://memgraph.com]
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

EOF
echo "$types" >> index.d.ts
