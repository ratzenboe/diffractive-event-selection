import sys

from modules.utils import get_output_paths


class logger(object):
    """
    Writes output both to terminal and to file.
    """
    
    def __init__(self, input_elements):
        self.terminal = sys.stdout
        output_prefix, model_prefix = get_output_paths(input_elements)
        self.log = open(output_prefix + model_prefix + 'stdout.log', 'w')

    def write(self, message):
        self.terminal.write(message)
        self.log.write(message)

    def flush(self):
        #this flush method is needed for python 3 compatibility.
        #this handles the flush command by doing nothing.
        #you might want to specify some extra behavior here.
        pass
