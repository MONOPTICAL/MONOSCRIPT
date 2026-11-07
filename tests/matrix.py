#!/usr/bin/env python3
"""Перебор сочетаний: тип, откуда значение и куда оно идёт.

Обычный тест проверяет одну возможность, а баги живут на стыках: "v=" + a[0] падал,
хотя и склейка строк, и a[0] по отдельности работали. На каждое сочетание собирается
программа с заранее известным ответом.

    python3 tests/matrix.py ./ms [jit|compile|all]

Выход 0, если все сочетания верны. REJECT значит, что компилятор отверг верный код.
"""
import collections
import os
import subprocess
import sys
import tempfile

LITERAL = {'i1': '1', 'i8': '5', 'i16': '300', 'i32': '70000', 'i64': '5000000000', 'float': '1.5', 'f64': '0.1', 'char': "'x'", 'string': '"s"'}
TEXT = {'i1': 'true', 'i8': '5', 'i16': '300', 'i32': '70000', 'i64': '5000000000', 'float': '1.5', 'f64': '0.1', 'char': 'x', 'string': 's'}
DOUBLED = {'i8': '10', 'i16': '600', 'i32': '140000', 'i64': '10000000000', 'float': '3.0', 'f64': '0.2'}
NEGATED = {'i8': '-5', 'i16': '-300', 'i32': '-70000', 'i64': '-5000000000', 'float': '-1.5', 'f64': '-0.1'}
AS_I32 = {'i1': '1', 'i8': '5', 'i16': '300', 'i32': '70000', 'i64': '705032704', 'float': '1', 'f64': '0'}
INT_VALUE = {'i1': 1, 'i8': 5, 'i16': 300, 'i32': 70000, 'i64': 5000000000}
PROGRAM_LINES = ('=', )
PROGRAM_WORDS = ('yes', 'no', 'v', 'BAD')


def sources(t):
    """Имя источника: (код верхнего уровня, код в начале main, выражение)."""
    value = LITERAL[t]
    result = {
        'var': ('', f'|   {t} v = {value}', 'v'),
        'global': (f'{t} gv = {value}\n', '', 'gv'),
        'array': ('', f'|   array<{t}> a = [{value}]', 'a[0]'),
        'nested': ('', f'|   array<array<{t}>> n = [[{value}]]', 'n[0][0]'),
        'map': ('', f'|   map<string,{t}> m = {{"k": {value}}}', 'm["k"]'),
        'field': (f'[struct]S\n|   {t} f = {value}\n\n', '|   S s = S()', 's.f'),
        'field_array': (f'[struct]S\n|   array<{t}> f = [{value}]\n\n', '|   S s = S()', 's.f[0]'),
        'call': (f'[{t}]g()\n|   return {value}\n\n', '', 'g()'),
        'method': (f'[struct]S\n|   {t} f = {value}\n|   [{t}]get()\n|   |   return self.f\n\n', '|   S s = S()', 's.get()'),
    }
    if t not in ('i1', 'float', 'f64'):  # x ^= 1 выводит i1, а x ^= 0.1 выводит f64
        result['inferred'] = ('', f'|   v ^= {value}', 'v')
    return result


def contexts(t, src):
    """Имя контекста: (код верхнего уровня, тело main, строка, которая должна быть в выводе)."""
    text = TEXT[t]
    value = LITERAL[t]
    result = {
        'join_right': ('', f'|   echo("=" + {src})', f'={text}'),
        'join_left': ('', f'|   echo({src} + "=")', f'{text}='),
        'assign': ('', f'|   {t} r = {src}\n|   echo("=" + r)', f'={text}'),
        'assign_inferred': ('', f'|   r ^= {src}\n|   echo("=" + r)', f'={text}'),
        'reassign': ('', f'|   {t} r = {src}\n|   r = {src}\n|   echo("=" + r)', f'={text}'),
        'param': (f'[{t}]id({t}: x)\n|   return x\n\n', f'|   echo("=" + id({src}))', f'={text}'),
        'method_arg': (f'[struct]Y\n|   [{t}]id({t}: x)\n|   |   return x\n\n', f'|   Y y = Y()\n|   echo("=" + y.id({src}))', f'={text}'),
        'in_array': ('', f'|   array<{t}> b = [{src}]\n|   echo("=" + b[0])', f'={text}'),
        'in_map': ('', f'|   map<string,{t}> b = {{"z": {src}}}\n|   echo("=" + b["z"])', f'={text}'),
        'to_element': ('', f'|   array<{t}> b = [{value}]\n|   b[0] = {src}\n|   echo("=" + b[0])', f'={text}'),
        'to_map': ('', f'|   map<string,{t}> b = {{}}\n|   b["z"] = {src}\n|   echo("=" + b["z"])', f'={text}'),
        'to_field': (f'[struct]Z\n|   {t} q = {value}\n\n', f'|   Z z = Z()\n|   z.q = {src}\n|   echo("=" + z.q)', f'={text}'),
        'cstr': ('', f'|   echo(cstr({src}))', text),
        'equal': ('', f'|   if {src} == {src}\n|   |   echo("yes")', 'yes'),
        'not_equal': ('', f'|   if {src} != {src}\n|   |   echo("BAD")\n|   else\n|   |   echo("no")', 'no'),
        'for_element': ('', f'|   array<{t}> b = [{src}, {src}]\n|   for e in b\n|   |   echo("=" + e)', f'={text}'),
    }

    if t != 'char':
        result['equal_text'] = ('', f'|   if {src} == "{text}"\n|   |   echo("yes")', 'yes')

    if t == 'char':
        result['cast_int'] = ('', f'|   echo("=" + {src}->i32)', '=120')
        result['less'] = ('', f'|   if {src} < {src}\n|   |   echo("BAD")\n|   else\n|   |   echo("no")', 'no')
        result['map_key'] = ('', f'|   map<char,string> b = {{}}\n|   b[{src}] = "v"\n|   echo(b[{src}])', 'v')

    if t not in ('string', 'char'):
        if t not in ('i64', 'float', 'f64'):
            result['to_i64'] = ('', f'|   i64 w = {src}\n|   echo("=" + w)', f'={AS_I32[t]}')
        # f64 хранит всё точнее, чем float: в обратную сторону нужен каст
        if t not in ('float', 'f64'):
            as_f64 = '5000000000.0' if t == 'i64' else AS_I32[t] + '.0'
            result['to_f64'] = ('', f'|   f64 w = {src}\n|   echo("=" + w)', f'={as_f64}')
            result['param_f64'] = ('[f64]pd(f64: x)\n|   return x\n\n', f'|   echo("=" + pd({src}))', f'={as_f64}')
            result['element_f64'] = ('', f'|   array<f64> b = [0.5]\n|   b[0] = {src}\n|   echo("=" + b[0])', f'={as_f64}')
            result['add_f64'] = ('', f'|   f64 d = 0.5\n|   f64 r = {src} + d\n|   echo("=" + r)', f'={as_f64[:-2]}.5')
        if t == 'float':
            result['to_f64'] = ('', f'|   f64 w = {src}\n|   echo("=" + w)', '=1.5')
            result['param_f64'] = ('[f64]pd(f64: x)\n|   return x\n\n', f'|   echo("=" + pd({src}))', '=1.5')
            result['add_f64'] = ('', f'|   f64 d = 0.25\n|   f64 r = {src} + d\n|   echo("=" + r)', '=1.75')
        if t not in ('float', 'f64'):
            whole = '5e+09' if t == 'i64' else AS_I32[t] + '.0'
            result['to_float'] = ('', f'|   float w = {src}\n|   echo("=" + w)', f'={whole}')
            result['cast_i32'] = ('', f'|   echo("=" + ({src})->i32)', f'={AS_I32[t]}')

    number = INT_VALUE.get(t)
    if number is not None:
        result['param_i64'] = ('[i64]pw(i64: x)\n|   return x\n\n', f'|   echo("=" + pw({src}))', f'={number}')
        result['return_i64'] = (f'[i64]rw({t}: x)\n|   return x\n\n', f'|   echo("=" + rw({src}))', f'={number}')
        result['element_i64'] = ('', f'|   array<i64> b = [0]\n|   b[0] = {src}\n|   echo("=" + b[0])', f'={number}')
        result['literal_i64'] = ('', f'|   array<i64> b = [{src}]\n|   echo("=" + b[0])', f'={number}')
        result['field_i64'] = ('[struct]W\n|   i64 q = 0\n\n', f'|   W z = W()\n|   z.q = {src}\n|   echo("=" + z.q)', f'={number}')
        result['map_i64'] = ('', f'|   map<string,i64> b = {{}}\n|   b["z"] = {src}\n|   echo("=" + b["z"])', f'={number}')
        result['add_i64'] = ('', f'|   i64 big = 1\n|   i64 r = {src} + big\n|   echo("=" + r)', f'={number + 1}')
        result['compare_i64'] = ('', f'|   i64 big = 0\n|   if {src} > big\n|   |   echo("yes")', 'yes')
        result['map_key'] = ('', f'|   map<i64,string> b = {{}}\n|   b[{src}] = "v"\n|   echo(b[{number}])', 'v')
        if number < 1000000:  # Дальше float теряет точность и точного ответа нет
            whole = f'{number}.0'
            result['add_float'] = ('', f'|   float f = 0.5\n|   float r = {src} + f\n|   echo("=" + r)', f'={number}.5')
            result['param_float'] = ('[float]pf(float: x)\n|   return x\n\n', f'|   echo("=" + pf({src}))', f'={whole}')
            result['element_float'] = ('', f'|   array<float> b = [0.5]\n|   b[0] = {src}\n|   echo("=" + b[0])', f'={whole}')
            result['field_float'] = ('[struct]W\n|   float q = 0.5\n\n', f'|   W z = W()\n|   z.q = {src}\n|   echo("=" + z.q)', f'={whole}')

    if t not in ('string', 'char'):
        # Каст сразу после значения и он же внутри выражения
        cast_value = 1 if t == 'float' else 0 if t == 'f64' else INT_VALUE[t] if t != 'i64' else 705032704
        result['cast_then_add'] = ('', f'|   i32 r = ({src}->i32) + 1\n|   echo("=" + r)', f'={cast_value + 1}')
        result['cast_argument'] = ('[i32]ic(i32: x)\n|   return x\n\n', f'|   echo("=" + ic({src}->i32))', f'={cast_value}')

    if t in DOUBLED:
        result['add'] = ('', f'|   {t} r = {src} + {src}\n|   echo("=" + r)', f'={DOUBLED[t]}')
        result['negate'] = ('', f'|   {t} r = -{src}\n|   echo("=" + r)', f'={NEGATED[t]}')
        result['less'] = ('', f'|   if {src} < {src}\n|   |   echo("BAD")\n|   else\n|   |   echo("no")', 'no')

    if t == 'i1':
        result['bool_add'] = ('', f'|   r ^= {src} + {src}\n|   echo("=" + r)', '=2')
        result['bool_sub'] = ('', f'|   i32 r = {src} - {src}\n|   echo("=" + r)', '=0')
        result['bool_mul'] = ('', f'|   i32 r = {src} * {src}\n|   echo("=" + r)', '=1')
        result['bool_div'] = ('', f'|   i32 r = {src} / {src}\n|   echo("=" + r)', '=1')
        result['bool_rem'] = ('', f'|   i32 r = {src} % {src}\n|   echo("=" + r)', '=0')
        result['bool_compare'] = ('', f'|   if {src} + {src} == 2\n|   |   echo("yes")', 'yes')
        result['condition'] = ('', f'|   if {src}\n|   |   echo("yes")', 'yes')
        result['not'] = ('', f'|   if !{src}\n|   |   echo("BAD")\n|   else\n|   |   echo("no")', 'no')
        result['and'] = ('', f'|   if {src} and {src}\n|   |   echo("yes")', 'yes')

    return result


def run_program(compiler, mode, path, binary):
    if mode == 'jit':
        return subprocess.run([compiler, 'run', path], capture_output=True, text=True, errors='replace', timeout=30)

    compiled = subprocess.run([compiler, '--compile', binary, path], capture_output=True, text=True, errors='replace', timeout=60)
    if compiled.returncode != 0:
        return compiled
    return subprocess.run([binary], capture_output=True, text=True, errors='replace', timeout=30)


def classify(result, expected):
    """Отладочная сборка печатает много лишнего, поэтому ищем ответ среди строк вывода."""
    lines = [line.strip() for line in result.stdout.splitlines()]
    if result.returncode < 0 or result.returncode >= 128:
        return 'CRASH', lines
    if expected in lines and result.returncode == 0:
        return 'OK', lines
    if result.returncode == 1 and 'Error' in result.stderr:
        return 'REJECT', lines
    return 'WRONG', lines


def run_mode(compiler, mode, workdir):
    counts = collections.Counter()
    failures = collections.defaultdict(list)
    path = os.path.join(workdir, 'case.ms')
    binary = os.path.join(workdir, 'case')

    for t in LITERAL:
        for source_name, (top, setup, src) in sources(t).items():
            for context_name, (context_top, body, expected) in contexts(t, src).items():
                program = top + context_top + '[i32]main()\n' + (setup + '\n' if setup else '') + body + '\n|   return 0\n'
                with open(path, 'w', encoding='utf-8') as file:
                    file.write(program)

                result = run_program(compiler, mode, path, binary)
                kind, lines = classify(result, expected)
                counts[kind] += 1
                if kind == 'OK':
                    continue

                if kind == 'REJECT':
                    detail = (result.stderr.strip().splitlines() or [''])[0]
                else:
                    answer = [line for line in lines if line.startswith(PROGRAM_LINES) or line in PROGRAM_WORDS][:2]
                    detail = f'вывод {answer}, ожидалось {expected}, код {result.returncode}'
                failures[(kind, context_name, t)].append((source_name, detail))

    print(f'{mode}: ' + ', '.join(f'{kind} {count}' for kind, count in sorted(counts.items())))
    for (kind, context_name, t), cases in sorted(failures.items()):
        names = ','.join(name for name, _ in cases)
        print(f'  {kind:6} {context_name:16} {t:6} {names}\n         {cases[0][1][:150]}')
    return not failures


def main():
    if len(sys.argv) not in (2, 3) or (len(sys.argv) == 3 and sys.argv[2] not in ('jit', 'compile', 'all')):
        print(__doc__)
        return 2

    compiler = os.path.abspath(sys.argv[1])
    mode = sys.argv[2] if len(sys.argv) == 3 else 'all'
    with tempfile.TemporaryDirectory(prefix='ms_matrix_') as workdir:
        modes = ('jit', 'compile') if mode == 'all' else (mode,)
        passed = [run_mode(compiler, m, workdir) for m in modes]
    return 0 if all(passed) else 1


if __name__ == '__main__':
    sys.exit(main())
    
#cmake --build build --target test-matrix