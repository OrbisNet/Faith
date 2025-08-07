from glob import glob as Glob
import os
import sys
from SCons.Script import DefaultEnvironment, Environment, Default, Builder, Command, Dir, Mkdir 
import platform

def GetEnvVar(Name:str) -> str:
    return os.environ.get(Name)

def create_fself_emitter(target, source, env):
    target_elf = str(target[0]).replace('.oelf', '.elf')
    return [target], [source[0], env.File(target_elf)]

def create_fself_generator(source, target, env, for_signature):
    return f"{env['CREATE_FSELF']} -in={source[0]} -out={str(target[0])} --lib={str(target[0]).replace('.oelf', '.prx')} --paid 0x3800000000000011"

TOOLCHAIN = GetEnvVar("OO_PS4_TOOLCHAIN")
GH_SDK = GetEnvVar("GOLDHEN_SDK")
PYRITE_SDK = GetEnvVar("PYRITE_SDK")
# Libraries
Libraries = []
Libraries.append("-lSceLibcInternal")
Libraries.append("-lGoldHEN_Hook") 
Libraries.append("-lkernel")
Libraries.append("-lSceSysmodule")
Libraries.append("-lSceNet")
Libraries.append("-lSceSsl")
Libraries.append("-lSceHttp")
Libraries.append("-lPyrite")

# You probably dont need to touch anything below that

env = Environment(tools=['gcc','g++'], ENV=os.environ)

INTDIR = "build"
FINALDIR = "build/prx"

os.makedirs(INTDIR, exist_ok=True)
os.makedirs(FINALDIR, exist_ok=True)

create_fself_builder = Builder(
    generator=create_fself_generator,
    emitter=create_fself_emitter,
)

env.Append(BUILDERS={'CreateFSelf': create_fself_builder})
OO_Toolchain_Platform = None

env['MSVC_USE_SCRIPT'] = None

if platform.system() == "Linux":
    OO_Toolchain_Platform = "linux"
    env.Replace(CC="clang")
    env.Replace(CXX="clang++")
    env.Replace(LINK="ld.lld")


if platform.system() == "Windows":
    OO_Toolchain_Platform = "windows"
    env.Replace(CC="clang.exe")
    env.Replace(CXX="clang++.exe")
    env.Replace(LINK="ld.lld.exe")

if platform.system() == "Darwin":
    OO_Toolchain_Platform = "macos"
    env.Replace(CC="/usr/local/opt/llvm/bin/clang") # Not sure if this is right, dont got a mac to test, Incase I am wrong
    env.Replace(CXX="/usr/local/opt/llvm/bin/clang++") # Ping me in The discord!
    env.Replace(LINK="/usr/local/opt/llvm/bin/ld.lld")


# C Flags # Add the include of your library here
env.Append(CFLAGS=["--target=x86_64-pc-freebsd12-elf"])
env.Append(CFLAGS=["-fPIC"])
env.Append(CFLAGS=["-funwind-tables"])
env.Append(CFLAGS=[f"-D__PRX_BUILD__"])
env.Append(CFLAGS=[f"-D__USE_PRINTF__"])
env.Append(CFLAGS=[f"-Wall"])
env.Append(CFLAGS=[f"-isysroot {TOOLCHAIN}"])
env.Append(CFLAGS=[f"-I{TOOLCHAIN}/include"])
env.Append(CFLAGS=[f"-I{TOOLCHAIN}/include/bits"])
env.Append(CFLAGS=[f"-I{TOOLCHAIN}/include/sys"])
env.Append(CFLAGS=[f"-I{GH_SDK}/include"])

# Pyrite
env.Append(CFLAGS=[f"-I{PYRITE_SDK}/core"])

# C++ Flags
env.Append(CXXFLAGS=[env["CFLAGS"]])
env.Append(CXXFLAGS=[f"-isystem {TOOLCHAIN}/include/c++/v1"])

# Linker Flags
env.Append(LDFLAGS=["-m elf_x86_64"])
env.Append(LDFLAGS=["-pie"])
env.Append(LDFLAGS=[f"--script {TOOLCHAIN}/link.x"])
env.Append(LDFLAGS=["-eh-frame-hdr"])
env.Append(LDFLAGS=["-e _init"])

# library Imports # Change this if you want to add new libaries to your project
env.Append(LDFLAGS=[f"-L{TOOLCHAIN}/lib"])
env.Append(LDFLAGS=[f"-L{GH_SDK}"])
env.Append(LDFLAGS=[f"-L{PYRITE_SDK}"])

env.Append(LIBS=Libraries)

SourceFiles =  [y for x in os.walk("Source") for y in Glob(os.path.join(x[0], '*.cpp'))]

obj_files = []
for src in SourceFiles:
    src_name = os.path.splitext(os.path.basename(str(src)))[0]
    obj_path = f'{INTDIR}/{src_name}.o'
    obj = env.Object(target=obj_path, source=src)
    obj_files.append(obj)

# Link the ELF file
output_name = os.path.basename(os.getcwd())
elf_target = f"{FINALDIR}/{output_name}.elf"
crt_obj = f"{GH_SDK}/build/crtprx.o"

link_cmd = f"{env['LINK']} {crt_obj} $SOURCES -o $TARGET {' '.join(env['LDFLAGS'])} {' '.join([f'-l{lib[2:]}' for lib in Libraries])}"

elf_file = env.Command(
    target=elf_target,
    source=obj_files,
    action=link_cmd
)

print("Creating PRX...")
env['CREATE_FSELF'] = f"{TOOLCHAIN}/bin/{OO_Toolchain_Platform}/create-fself"  


oelf_target = f"{FINALDIR}/{output_name}.oelf"

# Create the final PRX file
prx_file = env.CreateFSelf(
    target=oelf_target,
    source=elf_file
)

Default(prx_file)