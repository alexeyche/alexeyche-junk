struct Point {
    x:  int,
    y:  int
}

impl Point {
    fn neighbors() -> ~[Point] {
        return ~[Point { x: self.x - 1, y: self.y - 1 },
                 Point { x: self.x    , y: self.y - 1 },
                 Point { x: self.x + 1, y: self.y - 1 },
                 Point { x: self.x + 1, y: self.y     },
                 Point { x: self.x + 1, y: self.y + 1 },
                 Point { x: self.x    , y: self.y + 1 },
                 Point { x: self.x - 1, y: self.y + 1 },
                 Point { x: self.x - 1, y: self.y     }];
    }
}

struct LifeMap {
    width:  int,
    height: int,
    data:   ~[bool]
}

fn LifeMap(width: int, height: int) -> ~LifeMap {
    return ~LifeMap {
        width:  width,
        height: height,
        data:   vec::to_mut(vec::from_elem((width * height) as uint, false))
    };
}

impl LifeMap {
    fn get(p: &Point) -> bool {
        let wx = (p.x + self.width) % self.width;
        let wy = (p.y + self.height) % self.height;

        return self.data[(wy * self.width) + wx];
    }

    fn set(p: &Point, v: bool) {
        let wx = (p.x + self.width) % self.width;
        let wy = (p.y + self.height) % self.height;

        self.data[(wy * self.width) + wx] = v;
    }

    fn live_neighbors(p: &Point) -> int {
        let mut count = 0;

        for vec::each(p.neighbors()) |n| {
            if self.get(n) {
                count += 1;
            }
        }

        return count;
    }

    fn survives(p: &Point) -> bool {
        let n = self.live_neighbors(p);

        if self.get(p) {
            match n {
                2 | 3 => true,
                _ => false
            }
        } else {
            match n {
                3 => true,
                _ => false
            }
        }
    }

    fn render() {
        for int::range(0, self.height) |y| {
            for int::range(0, self.width) |x| {
                match self.get(&Point { x: x, y: y }) {
                    true => io::print("*"),
                    false => io::print("-")
                }
            }
            io::print("\n");
        }
        io::print("\n");
    }
}

fn evolve(curr: &LifeMap, next: &LifeMap) {
    for int::range(0, curr.height) |y| {
        for int::range(0, curr.width) |x| {
            let p = Point { x: x, y: y };
            match curr.survives(&p) {
                true => next.set(&p, true),
                false => next.set(&p, false)
            }
        }
    }
}

fn main() {
    let mut curr = LifeMap(4, 4);
    let mut next = LifeMap(4, 4);
    
    curr.set(&Point { x: 1, y: 1 }, true);
    curr.set(&Point { x: 2, y: 1 }, true);
    curr.set(&Point { x: 3, y: 1 }, true);

    for 4.times {
        curr.render();
        evolve(curr, next);
        curr <-> next;
    }
}
