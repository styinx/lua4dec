import sys
import struct
from pathlib import Path
from subprocess import run, CalledProcessError


def main(argv: list[str] = sys.argv[1:]):
    if len(argv) != 1:
        print('Provide an unmunged script file as an argument.')
        return 1

    script = Path(argv[0])
    lua_code = Path(script).with_suffix('.out')

    with script.open('rb') as blob:
        assert(blob.read(4).decode() == 'ucfb')
        _ = blob.read(4) # ucfb size
        assert(blob.read(4).decode() == 'scr_')
        _ = blob.read(4) # scr_ size
        assert(blob.read(4).decode() == 'NAME')
        name_size = struct.unpack_from('<I', blob.read(4))[0]
        _ = blob.read(name_size + (4 - name_size % 4)) # script name
        assert(blob.read(4).decode() == 'INFO')
        info_size = struct.unpack_from('<I', blob.read(4))[0]
        _ = blob.read(info_size + (4 - info_size % 4))
        assert(blob.read(4).decode() == 'BODY')
        body_size = struct.unpack_from('<I', blob.read(4))[0]

        with lua_code.open('wb') as bytecode:
            bytecode.write(blob.read(body_size))

    try:
        decompile = run(['luadec_32.exe', lua_code.absolute()], capture_output=True, text=True, check=True)
        print(decompile.stdout)
        return decompile.returncode

    except CalledProcessError as e:
        print("Program failed!")
        print("Exit code:", e.returncode)
        print("STDOUT:", e.stdout)
        print("STDERR:", e.stderr)
        return 3

    except FileNotFoundError:
        print("Executable not found!")
        return 2

    except Exception as e:
        print("Unexpected error:", e)
        return 1


if __name__ == '__main__':
    sys.exit(main())