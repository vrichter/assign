# assign
Uses preferences of participants to assign them to groups.

## build
```bash
git clone https://github.com/vrichter/assign.git && cd assign && mkdir build && cd build && cmake ..&& make
```

## usage

### Simple assignment with ``assign``
The application ``assign`` reads csv information from stdin and prints group assignments in csv
format to stdout.

#### Print example csv to stdout
```bash
> assing -e
jack,1,5,6,2,3
jill,6,2,4,6,2
paul,3,3,5,2,1
mila,2,9,7,4,3
jenn,5,6,3,7,1
```

#### Create and print an assignment for the example csv
```bash
> assing -e | assign
jack,3
jill,1
paul,4
mila,0
jenn,2
```

#### Create and print an assignment and costs for the example csv
```bash
> assign -e | assign --costs
jack,3,2
jill,1,2
paul,4,1
mila,0,2
jenn,2,3
```

### Extended assignment with ``assign_multiple``

The application subsumes the fuctionality of assign but has two additional commandline parameters
``--multiple <arg>`` and ``--exclusive <arg>`` for assignments to multiple groups with mutual
exclusions.

#### Assignment to multiple separate groups
The passed preferences can be split into two separate assignments.
```bash
> assign -e | assign_multiple --multiple 2
jack,0,1
jill,1,2
paul,1,2
mila,0,1
jenn,0,0
```
In this case the participants are assigned to one of two groups according to their first two
references and one of three groups according to the remaining preferences.

#### Costs
All costs are printed _after_ all assignments.
```bash
> assign -e | assign_multiple --multiple 2 --costs
jack,0,1,1,2
jill,1,2,2,2
paul,1,2,3,1
mila,0,1,2,4
jenn,0,0,5,3
```

#### Mutual exclusion
When assigning participants to multiple groups mutual exclusive group combinations can be considered.
```bash
> assign -e | assign_multiple --multiple 2 --costs --exclusive 0-1
jack,0,2,1,3
jill,1,0,2,4
paul,1,1,3,2
mila,0,2,2,3
jenn,0,0,5,3
```
In this case ``--exclusive 0-1`` prevents the application from assigning participants to groups
0 and 1 at the same time. This is realized by increasing the costs of mutually exclusive combinations
and is not guaranteed to find a globally optimal solution (only a local minimum is found).

### Example:
Jack, Jill, Paul, Mila and Jenn need to be assigned to two Tutorials and three Seminars.

We collect their preferences for each assignments in csv files and assign them to groups
considering their wishes.
```bash
> cat tutorial.csv
jack,1,5
jill,6,2
paul,3,3
mila,2,9
jenn,5,6
```
```bash
> cat seminar.csv
jack,6,2,3
jill,4,6,2
paul,5,2,1
mila,7,4,3
jenn,3,7,1
```
```bash
> cat tutorial.csv | assign
jack,0
jill,1
paul,1
mila,0
jenn,0
```
```bash
> cat seminar.csv | assign
jack,1
jill,2
paul,2
mila,1
jenn,0
```

Now we find out that our first tutorial and second seminar happen at the same time so participants
can not attend both at the same time.

We combine the preferences to a big list and use ``assign_multiple`` for a heuristic assignment.

```bash
> join -t , <(sort tutorial.csv) <(sort seminar.csv)
jack,1,5,6,2,3
jenn,5,6,3,7,1
jill,6,2,4,6,2
mila,2,9,7,4,3
paul,3,3,5,2,1
```
```bash
> join -t , <(sort tutorial.csv) <(sort seminar.csv) | ./assign_multiple --multiple 2 --exclusive 0-1
jack,0,2
jenn,0,0
jill,1,0
mila,0,2
paul,1,1
```
Due to the join the result is now sorted by id.




