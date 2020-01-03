// a trie (data structure), used for autocomplete
class Trie {
    constructor() {
        this.root = new Node(0);
    }

    // add a key-value pair to the trie
    add(k, v) {
        this.root.add(k, v);
    }

    // get a list of all values that match this key exactly
    get(k) {
        return this.root.get(k);
    }

    // get a list of all values with this key as a prefix
    getAll(k, options) {
        if (options === undefined) {
            options = {};
        }
        let limit = options.limit;

        let res = {};
        this.root.getAll(k.toLowerCase(), res, "", limit);
        return res;
    }
}

// a Trie node
class Node {
    constructor(level, parent) {
        this.level = level;
        this.children = {};
        this.parent = parent;
        this.values = [];
    }
    // add a child node based on key and values
    add(k, v) {
        if (k.length > this.level) {
            // if the key length is greater, it belongs in a child node
            let char = k[this.level];

            // if there is no child node with the correct letter, create one
            if (!(char in this.children)) {
                this.children[char] = new Node(this.level + 1, this);
            }
            // add the key-value pair to the child
            this.children[char].add(k, v);
        } else {
            // if the key length is not greater, the value
            // must belong in this node (base case)
            this.values.push(v)
        }
    }
    // get a list of values based on the key
    get(k) {
        if (k.length > this.level) {
            // if the key length is greater, it belongs in a child node
            let char = k[this.level];
            if (!(char in this.children)) {
                return undefined;
            } else {
                return this.children[char].get(k);
            }
        } else {
            // if the key length is not greater, the value
            // must belong in this node (base case)
            return this.values;
        }
    }

    // get all values beginning with this prefix. only get up  to `limit` of them
    getAll(k, res, prefix, limit) {
        if (k.length > this.level) {
            // if the key length is greater then the key is not a prefix of this
            let char = k[this.level];
            if (char in this.children) {
                limit = this.children[char].getAll(k, res, prefix + char, limit);
            }
        } else {
            // else return value from this up to the limit
            if (this.values.length > 0) {
                res[prefix]=[]
                for (let value of this.values) {
                    if (limit === 0) {
                        return limit;
                    }
                    res[prefix].push(value);
                    limit--;
                }
            }
            // then recursively call children until limit is reached
            for (let char in this.children) {
                if (limit === 0) {
                    return limit;
                }
                limit = this.children[char].getAll(k, res, prefix + char, limit);
            }
        }
        return limit;
    }
}

exports.Trie = Trie;
