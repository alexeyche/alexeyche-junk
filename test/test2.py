


def permutations(string, step = 0):

    # if we've gotten to the end, print the permutation
    if step == len(string):
        print "".join(string)
        return
    print step, string
    # everything to the right of step has not been swapped yet
    for i in range(step, len(string)):
    	print i, step
        # copy the string (store as array)
        string_copy = [character for character in string]

        # swap the current index with the step
        string_copy[step], string_copy[i] = string_copy[i], string_copy[step]

        # recurse on the portion of the string that has not been swapped yet (now it's index will begin with step + 1)
        permutations(string_copy, step + 1)




permutations("abcd")