# Подставляет дату и время сборки в src/build_info.cpp (макрос BUILD_TIMESTAMP).
#
# Define добавляется только этому одному файлу через build middleware:
# меняется его командная строка — SCons пересобирает только build_info.cpp
# и перелинковывает прошивку. Глобальный -D в build_flags пересобирал бы
# на каждой сборке весь проект вместе с Arduino-ядром.
from datetime import datetime

Import("env")

stamp = datetime.now().strftime("%Y-%m-%d %H:%M")


def add_build_timestamp(env, node):
    return env.Object(
        node,
        CPPDEFINES=list(env["CPPDEFINES"]) + [("BUILD_TIMESTAMP", env.StringifyMacro(stamp))],
    )


env.AddBuildMiddleware(add_build_timestamp, "*build_info.cpp")
