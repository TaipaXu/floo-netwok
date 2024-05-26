#!/bin/bash

OUTPUT_QRC="./src/web.qrc"

echo "<RCC>" > "$OUTPUT_QRC"
echo "    <qresource prefix=\"/web\">" >> "$OUTPUT_QRC"

find ./web -type f | while read -r file; do
    relative_path="${file#./web/}"
    echo "        <file alias=\"$relative_path\">.$file</file>" >> "$OUTPUT_QRC"
done

echo "    </qresource>" >> "$OUTPUT_QRC"
echo "</RCC>" >> "$OUTPUT_QRC"

echo "QRC file has been generated: $OUTPUT_QRC"
