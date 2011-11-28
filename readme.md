## Software for using audio hardware for neurophysiology research

The code contained in this repository implements a CoreAudio Audio Unit for performing online spike detection for electrophysiological recording experiments.

Modern audio recording hardware is inexpensive, high fidelity, and fast, and is supported by a rich and robust software ecosystem.  The plugin contained here is designed to do simple online detection of extracellular action potentials.  We are using it with the Logic software package from Apple, though it should work with any valid AudioUnit Host application.

The development of this code was supported by the National Science Foundation (grant IOS/0947777)
