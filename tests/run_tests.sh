#!/usr/bin/env bash
set -euo pipefail

COMPILER=./bin/CCL
INPUT_DIR=tests/inputs
EXPECTED_DIR=tests/expected

pass=0
fail=0

for src in "$INPUT_DIR"/*.c; do
    name=$(basename "$src" .c)
    expected="$EXPECTED_DIR/$name.out"

    echo -n "Test $name: "

    # 1) 编译并直接运行
    if ! "$COMPILER" "$src" --run > /dev/null 2>&1; then
        echo "❌ crash or non-zero exit"
        ((fail++))
        continue
    fi

    # 2) 捕获输出
    actual=$( "$COMPILER" "$src" --emit-ir /dev/stdout | lli )

    # 3) 对比
    if [[ -f "$expected" ]]; then
        want=$(<"$expected")
        if [[ "$actual" == "$want" ]]; then
            echo "✅"
            ((pass++))
        else
            echo "❌ output mismatch"
            echo "  want: '$want'"
            echo "  got:  '$actual'"
            ((fail++))
        fi
    else
        echo "⚠️  no expected/$name.out, skip diff"
    fi
done

echo
echo "Summary: $pass passed, $fail failed"
exit $fail
