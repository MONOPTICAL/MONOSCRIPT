#!/usr/bin/env python3
"""Exercise compiler argv boundaries and native string/input ABI in both modes."""
import pathlib
import subprocess
import sys
import tempfile

compiler = str(pathlib.Path(sys.argv[1]).resolve())


def run(args, **kwargs):
    return subprocess.run(args, capture_output=True, text=True, timeout=30, **kwargs)


with tempfile.TemporaryDirectory(prefix="ms paths ") as directory:
    root = pathlib.Path(directory)
    source = root / "hello source.ms"
    source.write_text('[i32]main()\n|   echo("hello")\n|   return 0\n')
    # These characters must remain filename bytes, including in llc and gcc argv.
    for name in ["output with spaces", "output'quote", "output;echo INJECTED", "output$(echo INJECTED)", "output`echo INJECTED`"]:
        binary = root / name
        built = run([compiler, "--compile", str(binary), str(source)])
        assert built.returncode == 0, (name, built.stdout, built.stderr)
        executed = run([str(binary)])
        assert executed.returncode == 0 and executed.stdout == "hello\n", executed
    source.write_text('[i32]main()\n|   result<string> r = input()\n|   if !r.ok\n|   |   echo("error " + r.code)\n|   |   echo(r.error)\n|   |   return 0\n|   string s = r.value\n|   echo("" + len(s))\n|   echo(s)\n|   return 0\n')
    binary = root / "input"
    built = run([compiler, "--compile", str(binary), str(source)])
    assert built.returncode == 0, built.stderr
    for command in [[compiler, "run", str(source)], [compiler, "run", str(source), "--offOptimization"], [str(binary)]]:
        for data, expected in [("abc\n", "3\nabc\n"), ("\n", "0\n\n"), ("", "0\n\n"), ("x" * 200 + "\n", "200\n" + "x" * 200 + "\n"), ("abc\r\n", "3\nabc\n")]:
            result = run(command, input=data)
            assert result.returncode == 0 and result.stdout.startswith(expected), (command, result)
        result = run(command, input="a\0b\n")
        assert result.returncode == 101 and "strings cannot contain a NUL byte" in result.stderr, result
        result = run(command, input="x" * 65535 + "\n")
        assert result.returncode == 0 and result.stdout.startswith("65535\n" + "x" * 65535 + "\n"), result
        result = run(command, input="x" * 65536 + "\n")
        assert result.returncode == 0 and result.stdout.startswith("error 2\ninput line is too long\n"), result
print("CLI paths and input ABI passed")
