#!/bin/bash

set -Eeuo pipefail
script_dir="$(cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd)"

cd "$script_dir"

for file in *.ts; do
  echo "Testing $file..."
  npx tsc "$file" && node "$(basename $file .ts).js"
done
