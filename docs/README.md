# Документация PharmDelivery

Документация на русском языке в формате LaTeX. Основной файл — `documentation.tex`.
Содержание описывает реализацию модели в этой версии проекта.
Дата документации — 10 октября 2026 года.

## Содержание

- `sections/01-overview.tex` — назначение, возможности и границы проекта.
- `sections/02-running.tex` — зависимости, сборка, аргументы и запуск.
- `sections/03-architecture.tex` — архитектура, карта файлов и состояние модели.
- `sections/04-model.tex` — дневной цикл, спрос, склад, доставка, скидки и финансы.
- `sections/05-api.tex` — интерфейс C++, сущности, результаты и пример интеграции.
- `sections/06-formats.tex` — входной CSV и справочник JSON-отчёта.
- `sections/07-testing.tex` — тесты, диагностика, ограничения и сопровождение.

## Сборка PDF

Нужны установленный TeX Live или MiKTeX, русский язык для Babel, шрифты T2A
и пакеты `iftex`, `geometry`, `amsmath`, `amssymb`, `array`, `booktabs`,
`longtable`, `tabularx`, `xcolor`, `listings`, `tikz`, `enumitem`, `fancyhdr`,
`microtype`, `xurl`, `hyperref`. Для pdfLaTeX рекомендуются масштабируемые
кириллические шрифты CM-Super. `latexmk` необязателен.

```bash
cd /home/toster/prac-backend/docs
bash build.sh
```

Результат: `build/documentation.pdf`. Вспомогательные файлы тоже находятся
в `build/`, которая исключена из Git локальным `.gitignore`.
Скрипт использует `latexmk`, если он установлен, иначе делает три прохода
компилятора для оглавления и перекрёстных ссылок. Shell escape не требуется.

По умолчанию используется pdfLaTeX. Для XeLaTeX или LuaLaTeX дополнительно
нужны `fontspec` и установленные шрифты Noto Serif, Noto Sans, Noto Sans Mono:

```bash
LATEX_ENGINE=xelatex bash build.sh
# или
LATEX_ENGINE=lualatex bash build.sh
```

Ручная сборка через pdfLaTeX из этой директории:

```bash
mkdir -p build
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error \
  -output-directory=build documentation.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error \
  -output-directory=build documentation.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error \
  -output-directory=build documentation.tex
```

Сборка PDF проверена командой `bash build.sh` через pdfLaTeX 10 октября 2026 года.
Готовый PDF и вспомогательные файлы не включаются в Git: в репозитории хранятся
исходники документации. Проверки приложения описаны в разделе 7.

## Редактирование

Изменяйте соответствующие файлы в `sections/`; оформление, настройки шрифтов,
листингов и титульного листа находятся в `documentation.tex`.
При обновлении кода пересмотрите параметры, алгоритмы и таблицы CSV/JSON,
затем пересоберите PDF. Исходники не зависят от внешних изображений и абсолютных
путей включения: директорию `docs/` можно перенести целиком.
