# assign
Uses preferences of participants to assign them to groups.

## build
```bash
git clone https://github.com/vrichter/assign.git && cd assign && mkdir build && cd build && cmake ..&& make
```

## usage
The application reads csv information from stdin and prints group assignments in json format to stdout.

### Print example csv to stdout
```bash
> assing -e
Jack,Bauer,jack,1,5,6,2,3
Jill,Bill,jill,6,2,4,6,2
Paul,Walker,paul,3,3,5,2,1
Will,Smith,will,2,9,7,4,3
Jenn,When,jenn,5,6,3,7,1
```

### Create and print an assignment for the example csv
```bash
> assing -e | assign
{
    "assignment": [
        {
            "first_name": "Jack",
            "last_name":  "Bauer",
            "id":         "jack",
            "group":      3
        },
        {
            "first_name": "Jill",
            "last_name":  "Bill",
            "id":         "jill",
            "group":      1
        },
        {
            "first_name": "Paul",
            "last_name":  "Walker",
            "id":         "paul",
            "group":      4
        },
        {
            "first_name": "Will",
            "last_name":  "Smith",
            "id":         "will",
            "group":      0
        },
        {
            "first_name": "Jenn",
            "last_name":  "When",
            "id":         "jenn",
            "group":      2
        }
    ]
}
```

