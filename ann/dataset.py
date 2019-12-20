import os
from os.path import join as pj
import h5py

from urllib.request import urlretrieve

def get_workdir():
    return pj(os.environ["HOME"], "ann_workdir")



def get_dataset_file(dataset):
    workdir = get_workdir()
    if not os.path.exists(workdir):
        os.mkdir(workdir)
    data_dir = pj(workdir, "data")

    if not os.path.exists(data_dir):
        os.mkdir(data_dir)
    return pj(data_dir, "{}.hdf5".format(dataset))


def get_dataset(which):
    hdf5_file = get_dataset_file(which)

    if not os.path.exists(hdf5_file):
        src_url = "http://ann-benchmarks.com/{}.hdf5".format(which)
        print("downloading {} -> {}...".format(src_url, hdf5_file))

        urlretrieve(src_url, hdf5_file)

    hdf5_f = h5py.File(hdf5_file)
    return hdf5_f
