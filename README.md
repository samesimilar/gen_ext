# gen_ext
## A template for generating PureData externals from exported gen~ code.

gen_ext is a wrapper that lets you compile code exported from a Max gen~ object into an "external" object that can be loaded into a PureData patch.

This project is intended to generate externals for use on the [Organelle](https://www.critterandguitari.com/organelle) synth, however it is not customized specifically for this platform. Simply, thus far I have limited my testing to the Organelle.

At this stage, gen_ext is still an experimental project. My knowledge of gen~ is fairly naive and limited, so there may be some things that don't work. 

### Things that don't work

* I'm not sure how fft inputs are handled, so it probably doesn't work (e.g. for spectral delay implementations)

This has been tested with gen~ code exported from Max 8.3.2.

### How to use gen_ext

1. In Max 8, send an `exportcode` message to your `gen~` object, to generate your c++ dsp code-base.
2. Create a new project folder and then copy the contents of this repository's `template` into it.
3. Copy the exported `gen~` dsp code, (including the `gen_dsp` folder) into the `gen` folder in the template. Make a note of the .cpp file that `gen~` exported.
4. Open `Makefile` in your project folder and edit the two variables at the top.
	- Make sure gen.name matches the name of the exported `.cpp` file. E.g. if the file was exported as `gen_exported.cpp`, then set `gen.name = gen_exported`.
	- Set lib.name to the name you would like for your new pd external object. E.g. if you set the name as `lib.name = myObject` then you can create your object in pd as `myObject~`.
5. If your gen~ object references buffers, see the section below to edit `gen_buffer.h`.
6. Copy your project folder to a folder on your Organelle.
7. On your Organelle, go to the project folder in your terminal and run `make all`.
8. Copy the generated `.pd_linux` to a patch folder or install it in the shared pd library.
9. Now you can use the new object in your pd patch.

#### Tip
If you send a `bang` to the new object, it will print out information about the gen~ parameters it accepts. To set a parameter on the object, send a `<symbol> <float>` to the first inlet, with `<symbol>` matching the parameter name.
	
#### Using buffers referenced in your gen~ object

If your gen~ object references a buffers, you need to define them so that they can be connected to arrays in Pd.

_Note: The names of your buffers must not contain spaces or non-alphanumeric characters, and must start with a letter. i.e. they must be valid C/C++ variable names._

1. Open the `gen_buffer.h` file.
2. Set the `WRAPPER_BUFFER_COUNT` macro to the number of buffers you wish to use. You can use up to 5.
3. Uncomment the `WRAPPER_BUFFER_NAME_x` macro definitions and set the name for each to the unique name referenced by gen~. 
	- The order doesn't matter but do not set greater or fewer buffer names then the number defined for `WRAPPER_BUFFER_COUNT`.
4. You can compile your external now and it will look for arrays in PureData with the given names.

_Tip: Send a `pdset originalname newname` message to the your object to switch in a Pd array with a different name for a buffer._

#### Setting a custom sample rate

The compiled external will accept an `sr` message that lets you reset the wrapped gen~ object with a new sample rate. This is useful if you are using the object in a sub-patch with a block~ object. A typical use would be to oversample the dsp in the sub-patch.

If no `sr` message is sent to the wrapped gen~ object, it will default to operating at the global sample rate for the system.
	
#### Attribution
The _gigaverb_ example included in this repository includes code exported from the _gen~.gigaverb_ example code included in Max 8.3.2. Cycling '74 attributes this implementation as being ported from an implementation by Juhana Sadeharju (kouhia@nic.funet.fi).

