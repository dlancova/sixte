## SIXTE - Simulation of X-ray Telescopes

SIXTE is a software package for X-ray telescope observation simulations developed
at the Remeis Observatory (ECAP). It allows to undertake instrument performance
analyses and to produce simulated event files for mission- and science studies.

The software strives to find a compromise between exactness of the simulation
and speed. For many cases, by using calibration files such as the PSF, RMF and
ARF, efficient simulations are possible at comparably high speed, even though
they include non-linear effects such as pile-up. Setups for some current and future
missions such as XMM-Newton, eROSITA or Athena are included in the package. Other setups
can be easily added by the user through specifying the main instrument
characteristics in a flexible, human-readable format.


Properties of X-ray sources to be simulated are described in
a detector-independent format called SIMPUT (SIMulation inPUT). This way the same input can be used for simulating
observations with all available instruments. The input files can be easily generated from standard data such as XSPEC spectral models or FITS images with tools provided with the SIXTE distribution. The input data scale well from single point sources up to very complicated setups. It lists the basic parameters of the sources such as their flux in a reference band, their positions in a table and can also reference images, spectra and lightcurves. The installation of SIMPUT is required for the SIXTE simulator [(https://github.com/thdauser/simput)](https://github.com/thdauser/simput).

---

**A summary of the implementation and performance is published in Astronomy & Astrophysics [(here)](https://www.aanda.org/articles/aa/abs/2019/10/aa35978-19/aa35978-19.html). If you use SIXTE for a publication, please cite it as `Dauser, Falkner, Lorenz, et al., 2019, A&A, 630, A66 `. For further information, installation instructions, a tutorial on how to use SIXTE, and to download the release versions and instrument files, visit the [SIXTE homepage](https://www.sternwarte.uni-erlangen.de/research/sixte/).**


Please send bug reports, problems and other comments to sixte-support@lists.fau.de.
Updates on the SIXTE package are announced via the [SIXTE mailing list](https://lists.fau.de/cgi-bin/listinfo/sixte-users/).
