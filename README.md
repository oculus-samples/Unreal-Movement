# Movement Sample

Unreal-Movement is a package that uses OpenXR’s tracking APIs to expose Meta Quest Pro’s Body Tracking (BT), Eye Tracking (ET), and Face Tracking (FT) capabilities.
With this package, developers can leverage tracking to populate VR environments with custom avatars that bring the expressiveness of users into the virtual environments that they create.

## Documentation
The documentation for this package can be found [here](https://developer.oculus.com/documentation/unreal/unreal-movement-sample/).
The API reference for this package can be found [here](https://developer.oculus.com/documentation/unreal/unreal-movement-implementation/).

## How to Use

### Load the project

First, ensure you have Git LFS installed by running this command:
```sh
git lfs install
```

Then, clone this repo using the "Code" button above, or this command:
```sh
git clone https://github.com/oculus-samples/Unreal-Movement
```

### Launch the project in the Unreal Editor using one of the following options.

#### Epic Games Launcher with MetaXR plugin

The easiest way to get started is to use the prebuilt Unreal Engine from the Epic Games Launcher, with MetaXR plugin.

1. Install the [Epic Games Launcher](https://www.epicgames.com/store/en-US/download)
2. In the launcher, install UE5 (recommended).
3. Download and install the MetaXR plugin from the [Unreal Engine 5 Integration download page](https://developer.oculus.com/downloads/package/unreal-engine-5-integration).
3. Launch the Unreal Editor
4. From "Recent Projects", click "Browse" and select `MovementSample.uproject`

#### Oculus Unreal fork

The Oculus Unreal fork will give you the most up to date integration of Oculus features. However, you must build the editor from its source.

Follow the instructions on [Accessing Unreal Engine source code on GitHub](https://www.unrealengine.com/en-US/ue-on-github) to obtain:
- an Epic account
- a GitHub account
- authorization to access the Unreal Engine source repository
Disregard instructions on downloading Epic’s Unreal Engine source code as you will be building the Oculus-VR fork of Unreal Engine source.

Make sure you have Visual Studio installed properly:
- Launch the Visual Studio Installer and click Modify for the Visual Studio version you want to use.
- Under the Workloads tab, click Game development with C++ if it isn’t checked and then click Modify.

1. Download the source code from the [Oculus-VR fork of Unreal Engine on GitHub](https://github.com/Oculus-VR/UnrealEngine).
2. Follow Epic’s instructions on [Building Unreal Engine from Source](https://docs.unrealengine.com/5.2/en-US/building-unreal-engine-from-source/) to complete the process.

Depending on your machine, the build may take awhile to complete.

# Licenses
The Unreal-Movement is released under the [Oculus SDK License](https://github.com/oculus-samples/Unreal-Movement/blob/main/LICENSE). The MIT License applies to only certain, clearly marked documents. If an individual file does not indicate which license it is subject to, then the Oculus License applies.

