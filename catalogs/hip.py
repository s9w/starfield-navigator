from astropy import units as u
from astropy.coordinates import SkyCoord
import math


# documentation from http://cdsarc.u-strasbg.fr/ftp/cats/I/239/ReadMe
# Byte-by-byte Description of file: hip_main.dat
# --------------------------------------------------------------------------------
#    Bytes Format Units   Label     Explanations
# --------------------------------------------------------------------------------
#        1  A1    ---     Catalog   [H] Catalogue (H=Hipparcos)               (H0)
#    9- 14  I6    ---     HIP       Identifier (HIP number)                   (H1)
#   42- 46  F5.2  mag     Vmag      ? Magnitude in Johnson V                  (H5)
#   52- 63  F12.8 deg     RAdeg    *? alpha, degrees (ICRS, Epoch=J1991.25)   (H8)
#   65- 76  F12.8 deg     DEdeg    *? delta, degrees (ICRS, Epoch=J1991.25)   (H9)
#   80- 86  F7.2  mas     Plx       ? Trigonometric parallax                 (H11)

with open("hip_main.dat", "r") as catalog_file, open("cc_hip1997.txt", "w") as cc:
    for line in catalog_file:
        hip = line[8:14].strip()
        mag = line[41:46]

        # filter empty position entries
        asc_str = line[51:63].strip()
        dec_str = line[64:76].strip()
        if asc_str == "" or dec_str == "":
            continue

        # filter negative parallaxes
        parallax = line[79:86]
        if float(parallax) < 0:
            continue

        # filter super-small parallax
        if float(parallax) < 0.01:
            continue

        # filter too far away stars
        distance_pc = 1000.0 / float(parallax)
        distance_ly = distance_pc * 3.26156
        if distance_ly > 70.0:
            continue

        c = SkyCoord(float(asc_str)*u.degree, float(dec_str)*u.degree, frame='icrs',  distance=distance_pc*u.pc)
        cc.write("{};{};{};{:.2f};{}\n".format(hip, c.galactic.l.degree, c.galactic.b.degree, c.galactic.distance.to(u.lyr).value, mag))

# from https://cdsarc.cds.unistra.fr/ftp/I/311/ReadMe
# Byte-by-byte Description of file: hip2.dat
# --------------------------------------------------------------------------------
#    Bytes Format Units    Label   Explanations
# --------------------------------------------------------------------------------
#    1-  6  I6    ---      HIP     Hipparcos identifier
#   16- 28 F13.10 rad      RArad   Right Ascension in ICRS, Ep=1991.25
#   30- 42 F13.10 rad      DErad   Declination in ICRS, Ep=1991.25
#   44- 50  F7.2  mas      Plx     Parallax
#  130-136  F7.4  mag      Hpmag   Hipparcos magnitude

with open("hip2.dat", "r") as catalog_file, open("cc_hip2007.txt", "w") as cc:
    for line in catalog_file:
        hip = line[0:6].strip()
        mag = line[129:136]

        # filter empty position entries
        asc_str = line[15:28].strip()
        dec_str = line[29:42].strip()
        if asc_str == "" or dec_str == "":
            continue

        # filter negative parallaxes
        parallax = line[43:50]
        if float(parallax) < 0:
            continue

        # filter super-small parallax
        if float(parallax) < 0.01:
            continue

        # filter too far away stars
        distance_pc = 1000.0 / float(parallax)
        distance_ly = distance_pc * 3.26156
        if distance_ly > 70.0:
            continue

        c = SkyCoord(math.degrees(float(asc_str))*u.degree, math.degrees(float(dec_str))*u.degree, frame='icrs',  distance=distance_pc*u.pc)
        cc.write("{};{};{};{:.2f};{}\n".format(hip, c.galactic.l.degree, c.galactic.b.degree, c.galactic.distance.to(u.lyr).value, mag))

