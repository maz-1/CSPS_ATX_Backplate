Import("env")

#
# Dump build environment (for debug)
# print(env.Dump())
#

env.Append(
  LINKFLAGS=[
    "-flto"
  ]
)
env.Append(
  CCFLAGS=[
    "-flto"
  ]
)
env.Append(
  CXXFLAGS=[
    "-flto"
  ]
)
