from astropy.coordinates import SkyCoord
from astropy import units as u
import math

count = 0
with open("hygdata_v3.csv", "r") as catalog_file, open("../starfield_navigator//cc_hyg.txt", "w") as cc:
    lines = [line for line in catalog_file]
    for line in lines[1:]:
        split = line.split(",")
        dist_str = split[9]
        if dist_str == "100000.0000":
            continue

        dist_ly = float(dist_str) * 3.262
        if dist_ly > 70.0:
            continue

        gliese_id = split[4]
        hip_id = split[1]
        proper = split[6]

        # Filter out the sun, literally
        if proper == "Sol":
            continue

        rarad = float(split[23])
        decrad = float(split[24])

        if hip_id == "" and gliese_id == "":
            print("empty id")
        
        id_str = "HIP_{}".format(hip_id)
        if hip_id == "":
            # id_str = gliese_id
            id_str = "GLIESE_{}".format(gliese_id.split()[1])

        c = SkyCoord(rarad*u.radian, decrad*u.radian, frame='icrs',  distance=dist_ly*u.lyr)
        cc.write("{};{};{};{:.2f}\n".format(id_str, c.galactic.l.degree, c.galactic.b.degree, c.galactic.distance.to(u.lyr).value))
        count += 1

print(count)
