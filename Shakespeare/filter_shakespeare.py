import argparse
import html.parser


class LineParser(html.parser.HTMLParser):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.stack = []
        self.lines = []
    
    def handle_starttag(self, tag, attrs):
        self.stack.append(tag)
    
    def handle_endtag(self, tag):
        popped = self.stack.pop()
        while popped != tag:
            popped = self.stack.pop()
    
    def isLine(self):
        return len(self.stack) >= 2 and self.stack[-1] == "a" and self.stack[-2] == "blockquote"
    
    def handle_data(self, data):
        if not self.isLine():
            return
        data = data.strip()
        if data.startswith("["):
            data = data.split("]")[-1].strip()
        if data:
            self.lines.append(data)


def main():
    parser = argparse.ArgumentParser(description="Filter a Shakespearean play (namely Hamlet) and keep only the direct speech\n\nInput should be the html body of a play's text from shakespeare.mit.edu")
    parser.add_argument("ifile", help="The raw input file")
    parser.add_argument("ofile", help="The output file")
    args = parser.parse_args()
    with open(args.ifile, "r") as ifile, open(args.ofile, "w") as ofile:
        lineParser = LineParser()
        lineParser.feed(ifile.read())
        ofile.write("\n".join(lineParser.lines))
    print("Done.")


if __name__ == "__main__":
    main()