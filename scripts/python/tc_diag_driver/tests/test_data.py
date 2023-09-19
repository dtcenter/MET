import pathlib


def get_gfs_dir():
    location_txt = pathlib.Path("tests/itests/test_data/gfs/data_location.txt")
    override_txt = pathlib.Path("tests/itests/test_data/gfs/override.txt")
    selected = location_txt
    if override_txt.exists():
        selected = override_txt

    with open(selected, "r") as in_file:
        return pathlib.Path(in_file.readline().strip())


GFS_DIR = get_gfs_dir()
ATCF_FILENAME = "tests/itests/test_data/atcf/aal092021.dat"