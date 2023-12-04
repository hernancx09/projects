import csv
from collections import deque
from time import sleep


class NTM:
    def __init__(self, filename):
        self.states = []
        self.alphabet = []
        self.tape_symbols = []
        self.transitions = {}
        self.start_state = None
        self.accept_state = None
        self.reject_state = None
        self.load_from_csv(filename)

    def load_from_csv(self, filename):
        with open(filename, mode='r', encoding='utf-8') as file:
            reader = csv.reader(file)
            lines = list(reader)

            self.states = lines[1]
            self.alphabet = lines[2]
            self.tape_symbols = lines[3]
            self.start_state = lines[4][0]
            self.accept_state = lines[5][0]
            self.reject_state = lines[6][0]

            for transition in lines[7:]:
                key = (transition[0], transition[1])
                value = (transition[2], transition[3], transition[4])
                if key not in self.transitions:
                    self.transitions[key] = []
                self.transitions[key].append(value)

    def simulate(self, input_string):
        initial_config = {'state': self.start_state, 'tape': list(input_string) + ['_'], 'head': 0, 'steps': 0, 'parent': None}
        configs = deque([initial_config])
        max_depth = 0

        while configs:
            config = configs.popleft()
            state, tape, head, steps = config['state'], config['tape'], config['head'], config['steps']
            max_depth = max(max_depth, config['steps'])

            if state == self.accept_state:
                return f"String accepted in {steps} transitions", config['steps'], self.get_configuration_path(config)
            if state == self.reject_state:
                continue

            current_symbol = tape[head] if head < len(tape) else '_'
            transitions = self.transitions.get((state, current_symbol), [])
            for new_state, new_symbol, direction in transitions:
                new_tape = tape.copy()
                new_tape[head] = new_symbol
                new_head = head + (1 if direction == 'R' else -1)
                new_config = {'state': new_state, 'tape': new_tape, 'head': new_head, 'steps': steps + 1, 'parent': config}
                configs.append(new_config)

        return f"String rejected in {max_depth} transitions",None, self.get_configuration_path(config)

    def get_configuration_path(self, config):
        path = []
        while config:
            path.append(self.format_configuration(config))
            config = config['parent']
        return path[::-1]

    def format_configuration(self, config):
        left_of_head = ''.join(config['tape'][:config['head']])
        right_of_head = ''.join(config['tape'][config['head']+1:])
        head_char = config['tape'][config['head']] if config['head'] < len(config['tape']) else '_'
        return f"{left_of_head}({config['state']},{head_char}){right_of_head}"

def main():
    choice = 'Y'
    ntm = NTM(str(input('Enter a csv file with NTM instructions... ')))
    while True:
            
        input_string = str(input('Enter a string to analyze in this NTM... '))
        result, depth, path = ntm.simulate(input_string)
        sleep(.2)
        print('analyzing...')
        sleep(.3)
        print(result)
        sleep(.3)
        if depth:
            print(f"Maximum depth reached: {depth}")
        if path:
            for step in path:
                print(step)
                sleep(.1)
        sleep(.3)
        cont = int(input('\nTo change NTM, enter 1\nTo quit program, enter 2\nTo continue inputing strings into the same NTM, enter any other number\n'))
        match cont:
            case 1:
                ntm = NTM(str(input('Enter a csv file with NTM instructions... ')))
                continue
            case 2:
                return
            case _:
                continue

if __name__ == "__main__":
    main()
