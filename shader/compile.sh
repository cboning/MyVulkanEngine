#!/bin/bash

SRC_DIR="src"
OUT_DIR="bin"

mkdir -p "$OUT_DIR"

for file in "$SRC_DIR"/*.{vert,frag,comp}; do
    # 跳过不存在的匹配
    [ -f "$file" ] || continue
    
    filename=$(basename "$file")
    name_no_ext="${filename%.*}"
    ext="${filename##*.}"

    # 首字母大写 + 文件类型后缀
    type_suffix="$(tr '[:lower:]' '[:upper:]' <<< ${ext:0:1})${ext:1}"
    out_file="$OUT_DIR/${name_no_ext}${type_suffix}.spv"

    glslc "$file" -o "$out_file" || exit 1
done

echo "All shaders compiled successfully."
