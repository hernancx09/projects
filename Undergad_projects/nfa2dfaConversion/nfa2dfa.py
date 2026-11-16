import csv
from tqdm import tqdm

class NFA(object):
    def __init__(self, file):
        self.input_file = file # get data source
        self.T = {} # initialize T
        with open(self.input_file) as file:   #read into file and extract data
            for i, line in tqdm(enumerate(file), desc='Reading NFA file...'):
               match i:
                    case 0:
                       self.name = line.rstrip().split(',')[0]
                       continue
                    case 1:
                        self.states = list(filter(None, line.rstrip().split(',')))
                        for state in self.states:
                            self.T[state] = []
                    case 2:
                        self.symbols = list(filter(None, line.rstrip().split(',')))
                    case 3:
                        self.start = line.rstrip().split(',')[0]
                    case 4:
                        self.accept = list(filter(None, line.rstrip().split(',')))
                    case _:
                        self.read_transition(line.rstrip())


    def read_transition(self, transition):
        values = transition.split(',')
        if len(values) < 3:
            return
        curr, input, next = values[0:3]
        if curr and input and next:
            if curr not in self.T:
                self.T[curr] = []
            self.T[curr].append((input, next))

    def epsilon_closure(self, state):
        if state == self.name:
            return []
        visited = {}
        visited[state] = 0
        frontier = [state]
        while (frontier):
            curr = frontier.pop()
            if (curr in visited and visited[curr] == 1):
                continue
            visited[curr] = 1
            for next in self.T[curr]:
                if (next[0] == '~'):
                    frontier.append(next[1])
        return list(visited.keys())


class DFA(object):
    def __init__(self):
        self.file = ''
        self.name = ''
        self.symbols = []
        self.start = ''
        self.accept = []
        self.states = []
        self.T = {}

    def write_transitions(self):
        self.transitions = []
        for state in self.states:
            for symbol in self.symbols:
                self.transitions.append([state, symbol, self.T[state][symbol]])

    def write_output(self):
        self.write_transitions()
        output_file = self.file[:-4] + '-DFA-Output.csv'
        if (len(self.accept) == 1):
            self.accept = [self.accept]
        with open(output_file, 'w') as file:
            writer = csv.writer(file)
            writer.writerow([self.name])
            writer.writerow(self.states)
            writer.writerow(self.symbols)
            writer.writerow([self.start])
            writer.writerow(self.accept)
            writer.writerows(self.transitions)

def get_closure(nfa):
    global closure
    closure = {}
    for state in nfa.states:
        if state != nfa.name:
            closure[state] = sorted(nfa.epsilon_closure(state))

def convert_2dfa(nfa, dfa):
    dfa_states = [dfa.start]
    frontier = [closure[nfa.start]]
    dfa_transitions = {}
    dfa_transitions['Err'] = {}
    while (frontier):
        curr = frontier.pop()
        if (len(curr) > 0):
            curr_str = '+'.join(curr)
        else:
            curr_str = ''.join(curr)
        dfa_transitions[curr_str] = {}
        for symbol in nfa.symbols:
            to_state = []
            for curr_state in curr:
                for next in nfa.T[curr_state]:
                    if (symbol == next[0]):
                        if (next[1] not in to_state):
                            to_state.append(next[1])

                if (to_state):
                    new_state = []
                    for state in to_state:
                        for closure_state in closure[state]:
                            if (closure_state not in new_state):
                                new_state.append(closure_state)

                    new_state = sorted(new_state)

                    if (len(new_state)>0):
                        new_state_str = '+'.join(new_state)
                    else:
                        new_state_str = ''.join(new_state)

                    dfa_transitions[curr_str][symbol] = new_state_str

                    if (new_state_str not in dfa_states):
                        dfa_states.append(new_state_str)
                        frontier.append(new_state)
                else:
                    new_state_str = 'Err'
                    if new_state_str not in dfa_states:
                        dfa_states.append(new_state_str)

    if ("Err" in dfa_states):
        for state in dfa_states:
            for symbol in nfa.symbols:
                if (symbol not in dfa_transitions[state]):
                    dfa_transitions[state][symbol] = 'Err'

    return dfa_states, dfa_transitions

def nfa2dfa(nfa, dfa):
    get_closure(nfa)
    dfa.file = nfa.input_file # same file same things
    dfa.name = nfa.name
    dfa.symbols = nfa.symbols
    if (len(closure[nfa.start]) > 1):
        dfa.start = '+'.join(closure[nfa.start])
    else:
        dfa.start = nfa.start
    dfa.states, dfa.T = convert_2dfa(nfa, dfa)
    for state in dfa.states:
        if (state[0] == '*'): # accepting that some have * for final state
            dfa.accept.append(state)
        else:
            states = state.split('+') # Creating the correct merges for states in DFA
            for s in states:
                if s in nfa.accept:
                    dfa.accept.append(state)
                    break
    dfa.write_output()


def main():
    for file in ['N1.csv', 'N3.csv', 'N4.csv']:
        print(f'Processing {file}...')
        nfa = NFA(file)
        dfa = DFA()
        nfa2dfa(nfa, dfa)
        print(f'Converted {file} to DFA')
        print('-' * 40)

if __name__ == '__main__':
    main()

