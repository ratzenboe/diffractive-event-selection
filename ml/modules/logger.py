import sys
import os
import errno

class logger(object):
    """
    Writes output both to terminal and to file.
    """
    
    def __init__(self, output_path):
        self.terminal = sys.stdout

        if not os.path.exists(output_path):
            print('Info: path {} does not exist so it will be created.'.format(
                output_path))
            try:
                os.makedirs(output_path)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise

        self.log = open(output_path + 'stdout.log', 'w')

    def write(self, message):
        self.terminal.write(message)
        self.log.write(message)

    def flush(self):
        pass

