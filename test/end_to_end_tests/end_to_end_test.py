import difflib
import os
import pathlib
import subprocess
from typing import TextIO

import pytest

# Should be in test/ when you run pytest or use --rootdir=test
canyon_compiler = os.path.join(os.getcwd(), "../build/canyon")
os.chdir("end_to_end_tests")
tests = os.path.join(os.getcwd(), "tests")


def canyon(infile: str, outfile: str, stdin: TextIO, stdout: TextIO, stderr: TextIO):
    return subprocess.Popen([canyon_compiler, infile, outfile], stdin=stdin, stdout=stdout, stderr=stderr)


def gcc(infile: str, outfile: str, stdin: TextIO, stdout: TextIO, stderr: TextIO):
    return subprocess.Popen(["gcc", infile, "-o", outfile], stdin=stdin, stdout=stdout, stderr=stderr)


def run(program: str, stdin: TextIO, stdout: TextIO, stderr: TextIO):
    return subprocess.Popen(program, stdin=stdin, stdout=stdout, stderr=stderr)


def discover_tests(directory: str):
    return [pathlib.Path(entry).name for entry in os.listdir(directory) if os.path.isdir(os.path.join(directory, entry))]


@pytest.mark.parametrize("test_name", discover_tests(os.path.join(tests, "success")))
def test_success(test_name, monkeypatch: pytest.MonkeyPatch, tmp_path: pathlib.Path):
    source = os.path.join(tests, "success", test_name)
    monkeypatch.chdir(tmp_path)
    pathlib.Path("output").mkdir(exist_ok=True)

    process = canyon(os.path.join(source, "main.canyon"), "main.c", None,
                     subprocess.PIPE, subprocess.PIPE)
    out, err = process.communicate()
    assert process.wait() == 0
    assert out.decode() == ""
    # assert err.decode() == ""

    process = gcc("main.c", "a.out", None, subprocess.PIPE, subprocess.PIPE)
    assert process.wait() == 0
    out, err = process.communicate()
    assert process.wait() == 0
    assert out.decode() == ""
    assert err.decode() == ""

    process = subprocess.Popen(
        "./a.out", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    assert process.wait() == 0

    exp_out_file = os.path.join(source, "expected/stdout")
    exp_err_file = os.path.join(source, "expected/stderr")
    try:
        with open(exp_out_file, "r") as exp_out:
            expected_stdout = exp_out.read()
    except FileNotFoundError:
        expected_stdout = ""
    try:
        assert out.decode() == expected_stdout
    except AssertionError:
        pathlib.Path(os.path.join(source, "failure")).mkdir(exist_ok=True)
        diff = difflib.unified_diff(
            expected_stdout.splitlines(), out.decode().splitlines())
        with open(os.path.join(source, "failure/stdout.diff"), "w") as fail_out:
            fail_out.write("\n".join(diff))
        raise

    try:
        with open(exp_err_file, "r") as exp_err:
            expected_stderr = exp_err.read()
    except FileNotFoundError:
        expected_stderr = ""
    try:
        assert err.decode() == expected_stderr
    except AssertionError:
        pathlib.Path(os.path.join(source, "failure")).mkdir(exist_ok=True)
        diff = difflib.unified_diff(
            expected_stderr.splitlines(), err.decode().splitlines())
        with open(os.path.join(source, "failure/stderr.diff"), "w") as fail_err:
            fail_err.write("\n".join(diff))
        raise


@pytest.mark.parametrize("test_name", discover_tests(os.path.join(tests, "canyon_failure")))
def test_failure(test_name, monkeypatch: pytest.MonkeyPatch, tmp_path: pathlib.Path):
    source = os.path.join(tests, "canyon_failure", test_name)
    monkeypatch.chdir(tmp_path)
    pathlib.Path("output").mkdir(exist_ok=True)

    main_source = os.path.join(source, "main.canyon")

    process = canyon(main_source, "main.c", None,
                     subprocess.PIPE, subprocess.PIPE)
    out, err = process.communicate()
    assert process.wait() != 0

    exp_out_file = os.path.join(source, "expected/stdout")
    exp_err_file = os.path.join(source, "expected/stderr")
    try:
        with open(exp_out_file, "r") as exp_out:
            expected_stdout = exp_out.read().format(main=main_source)
    except FileNotFoundError:
        expected_stdout = ""
    try:
        assert out.decode() == expected_stdout
    except AssertionError:
        pathlib.Path(os.path.join(source, "failure")).mkdir(exist_ok=True)
        diff = difflib.unified_diff(
            expected_stdout.splitlines(), out.decode().splitlines())
        with open(os.path.join(source, "failure/stdout.diff"), "w") as fail_out:
            fail_out.write("\n".join(diff))
        raise

    try:
        with open(exp_err_file, "r") as exp_err:
            expected_stderr = exp_err.read().format(main=main_source)
    except FileNotFoundError:
        expected_stderr = ""
    try:
        assert err.decode() == expected_stderr
    except AssertionError:
        pathlib.Path(os.path.join(source, "failure")).mkdir(exist_ok=True)
        diff = difflib.unified_diff(
            expected_stderr.splitlines(), err.decode().splitlines())
        with open(os.path.join(source, "failure/stderr.diff"), "w") as fail_err:
            fail_err.write("\n".join(diff))
        raise
