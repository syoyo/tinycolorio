import argparse
import PyOpenColorIO

def fit():
    print("bora")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
                    prog = 'fitting',
                    description = 'Polynomial fitting for ocio 1D and 3D lut')

    parser.add_argument('-i', '--input', nargs="?", type=argparse.FileType('r'), default=sys.stdin) 

    fit();
