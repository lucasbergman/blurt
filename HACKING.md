# Disorganized notes about development

## Dependencies and build setup

The network bit has two dependencies: the Mumble protocol buffer definition and
the Google protocol buffers support library. The proto definition gets copied
verbatim [from the main Mumble source
tree](https://github.com/mumble-voip/mumble/blob/src/Mumble.proto), most
recently on 2022-08-30 at commit d100ff1. For protocol buffers tooling and
support, we use [vcpkg](https://vcpkg.io/) with a
[manifest](https://vcpkg.readthedocs.io/en/latest/users/manifests/).

Note that the protocol buffers dependency is odd, because it's both a tool run
on the host (`protoc`) and a library used on the target (`libprotobuf*`). The
way that vcpkg hooks into the build process isn't really built for this
([example](https://github.com/microsoft/vcpkg/discussions/21601)) as far as I
can tell. So for now you're stuck running `vcpkg install --triplet x64-uwp`
once by hand before the full build will work.

The `.proto` file is in the project with a [custom build
tool](https://docs.microsoft.com/en-us/cpp/build/specifying-custom-build-tools)
that runs `protoc.exe` out of the vcpkg installation tree. The generated
source files appear in the project even though they don't exist in the source
tree, just like the source files generated from
[IDL](https://docs.microsoft.com/en-us/uwp/midl-3/) definitions.
