#!/usr/bin/env bash
set -euo pipefail

docs_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cd -- "$docs_dir"
latex_engine=${LATEX_ENGINE:-pdflatex}

case "$latex_engine" in
    pdflatex) latexmk_mode=-pdf ;;
    xelatex) latexmk_mode=-xelatex ;;
    lualatex) latexmk_mode=-lualatex ;;
    *)
        printf 'Поддерживаемые LATEX_ENGINE: pdflatex, xelatex, lualatex.\n' >&2
        exit 2
        ;;
esac

if ! command -v "$latex_engine" >/dev/null 2>&1; then
    printf 'Компилятор %s не найден. Установите LaTeX и зависимости из README.md.\n' "$latex_engine" >&2
    exit 127
fi

mkdir -p build
if command -v latexmk >/dev/null 2>&1; then
    latexmk "$latexmk_mode" -interaction=nonstopmode -halt-on-error \
        -file-line-error -outdir=build documentation.tex
else
    for pass in 1 2 3; do
        "$latex_engine" -interaction=nonstopmode -halt-on-error \
            -file-line-error -output-directory=build documentation.tex
    done
fi

printf 'Документация собрана: %s/build/documentation.pdf\n' "$docs_dir"
