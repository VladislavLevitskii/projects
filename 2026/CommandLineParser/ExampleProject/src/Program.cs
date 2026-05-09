using CommandLineParser;
using System.Text;

namespace ExampleUseApi
{
    public class SimpleHelpProvider : HelpProvider
    {
        private readonly List<OptionHelpInfo> _options;
        private readonly List<ParameterHelpInfo> _arguments;

        public SimpleHelpProvider(
            List<OptionHelpInfo> options, 
            List<ParameterHelpInfo> arguments, 
            ProgramHelpInfo programHelp) 
            : base(programHelp.longDescription, programHelp.shortDescription)
        {
            _options = options;
            _arguments = arguments;
        }

        public override string GetLongHelpText()
        {
            var sb = new StringBuilder();

            sb.AppendLine("============= Help =============");
            sb.AppendLine($"About program (long): {base.GetLongHelpText()}");
            sb.AppendLine($"About program (short): {base.GetShortHelpText()}");
            sb.AppendLine("=========================================");
            sb.AppendLine();

            sb.AppendLine("--- OPTIONS ---");
            foreach (var opt in _options)
            {
                sb.AppendLine($"Name: {opt.name}");
                sb.AppendLine($"Aliases: {string.Join(", ", opt.aliases)}");
                sb.AppendLine($"Parameters: {string.Join(", ", opt.parameterNames)}");
                sb.AppendLine($"Short help: {opt.shortDescription}");
                sb.AppendLine($"Long help: {opt.longDescription}");
                sb.AppendLine("-------------------------------");
            }

            sb.AppendLine();
            sb.AppendLine("--- ARGUMENTS ---");
            foreach (var arg in _arguments)
            {
                sb.AppendLine($"Name:        {arg.name}");
                sb.AppendLine($"Short help: {arg.shortDescription}");
                sb.AppendLine($"Long help: {arg.longDescription}");
                sb.AppendLine("-------------------------------");
            }

            return sb.ToString();
        }

        public override string GetShortHelpText()
        {
            var sb = new StringBuilder();
            sb.AppendLine("Toto je jen tupá zkrácená verze.");
            sb.AppendLine($"Program dělá: {base.GetShortHelpText()}");
            sb.AppendLine($"Počet voleb: {_options.Count}, Počet argumentů: {_arguments.Count}");
            return sb.ToString();
        }
    }

    internal class Program
    {
        public class ParserSchema
        {
            public CommandParser parser = null!;

            public OptionDefinition plainArgsDef = null!;
            
            public OptionDefinition formatOptionDef = null!;
            public OptionDefinition outputOptionDef = null!;
            public OptionDefinition portabilityOptionDef = null!;
            public OptionDefinition appendOptionDef = null!;
            public OptionDefinition verboseOptionDef = null!;
            public OptionDefinition helpOptionDef = null!;
            public OptionDefinition versionOptionDef = null!;

            public ParameterDefinition<string> fileParamDef = null!;
        }

        public static void InitParse(ParserSchema options)
        {
            options.formatOptionDef = new OptionDefinitionBuilder("format")
                .WithHelp(new HelpProvider(
                    longHelpText: "Specify output format, possibly overriding the format specified in the environment variable TIME.",
                    shortHelpText: "Specify output format."
                ))
                .WithAlias("-f", "--format")
                // Adding the parameter inline via lambda
                .AddParameter<string>("FORMAT", param => param
                    .IsRequired(true)
                    .WithHelp(new HelpProvider(longHelpText: "The format string.", shortHelpText: "Format string."))
                    .WithValidator(new DefaultValidators.StringValidator())
                    .WithParser(new DefaultParsers.StringParser())
                    .Build()
                )
                .Build();

            options.outputOptionDef = new OptionDefinitionBuilder("output")
                .WithHelp(new HelpProvider(
                    longHelpText: "Do not send the results to stderr, but overwrite the specified file.",
                    shortHelpText: "Write to file instead of stderr."
                ))
                .WithAlias("-o", "--output")
                // Adding the parameter inline via lambda
                .AddParameter<string>("FILE", param => param
                    .IsRequired(true)
                    .WithHelp(new HelpProvider(longHelpText: "The target file path.", shortHelpText: "File path."))
                    .WithValidator(new DefaultValidators.StringValidator())
                    .WithParser(new DefaultParsers.StringParser())
                    .WithDefault("results.txt")
                    .Build(out options.fileParamDef)
                )
                .Build();

            options.portabilityOptionDef = new OptionDefinitionBuilder("portability")
                .WithHelp(new HelpProvider(longHelpText: "Use the portable output format.", shortHelpText: "Portable output."))
                // '-' will be added implicitly before p
                .WithAlias("-p", "--portability")
                .Build();

            options.appendOptionDef = new OptionDefinitionBuilder("append")
                .WithHelp(new HelpProvider(longHelpText: "(Used together with -o.) Do not overwrite but append.", shortHelpText: "Append to file."))
                .WithAlias("-a", "--append")
                .Build();

            options.verboseOptionDef = new OptionDefinitionBuilder("verbose")
                .WithHelp(new HelpProvider(longHelpText: "Give very verbose output about all the program knows about.", shortHelpText: "Verbose output."))
                .WithAlias("-v", "--verbose")
                .Build();

            options.helpOptionDef = new OptionDefinitionBuilder("help")
                .WithHelp(new HelpProvider(longHelpText: "Print a usage message on standard output and exit successfully.", shortHelpText: "Print help."))
                .WithAlias("--help")
                .Build();

            options.versionOptionDef = new OptionDefinitionBuilder("version")
                .WithHelp(new HelpProvider(longHelpText: "Print version information on standard output, then exit successfully.", shortHelpText: "Print version."))
                .WithAlias("-V", "--version")
                .Build();

            options.parser = new CommandParserBuilder(plainArgsDelimiter: "--")
                .WithHelp(new HelpProvider(
                    longHelpText: "time [options] command [arguments...]",
                    shortHelpText: "time command"
                ))
                .WithOption(options.formatOptionDef)
                .WithOption(options.outputOptionDef)
                .WithOption(options.portabilityOptionDef)
                .WithOption(options.appendOptionDef)
                .WithOption(options.verboseOptionDef)
                .WithOption(options.helpOptionDef)
                .WithOption(options.versionOptionDef)

                .WithPlainArg<string>("command", param => param
                    .IsRequired(true)
                    .WithHelp(new HelpProvider(
                        longHelpText: "The command to run and time.",
                        shortHelpText: "Target command."
                    ))
                    .WithValidator(new DefaultValidators.StringValidator())
                    .WithParser(new DefaultParsers.StringParser())
                    .Build()
                )
                .WithCustomHelp((opts, args, progInfo) => new SimpleHelpProvider(opts, args, progInfo))
                .Build();
        }

        static void Main(string[] args)
        {
            ParserSchema parserSchema = new ParserSchema();
            InitParse(parserSchema);

            var help = parserSchema.parser.GetHelp();
            Console.WriteLine(help.GetLongHelpText());

            Console.WriteLine("Enter a mock 'time' command line to parse (e.g., -v -o results.txt -- ls -l):");
            string[] line = Console.ReadLine()?.Split() ?? Array.Empty<string>();

            while (line != null && line.Length > 0)
            {
                CommandParserResult result = parserSchema.parser.Parse(line);

                if (result)
                {
                    if (result.GetOption("help") != null)
                    {
                        Console.WriteLine("Mock Output: Printing Help Message...");
                        line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
                        continue;
                    }
                    if (result.HasOption(parserSchema.versionOptionDef))
                    {
                        Console.WriteLine("Mock Output: GNU time version 1.9 (mock)");
                        line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
                        continue;
                    }

                    var plainArgsResult = result.GetOption(parserSchema.plainArgsDef);
                    if (plainArgsResult != null)
                    {
                        // Retrieved by name
                        string targetCommand = plainArgsResult.GetArg<string>("command") ?? "";
                        Console.WriteLine($"\n[Executing Mock Command]: {targetCommand}");
                    }
                    else
                    {
                        Console.WriteLine("\nError: No command specified to time.");
                        line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
                        continue;
                    }

                    Console.WriteLine("--- Mock Time Results Configuration ---");

                    if (result.GetOption(parserSchema.verboseOptionDef) != null)
                        Console.WriteLine("* Verbosity: HIGH");

                    if (result.GetOption(parserSchema.portabilityOptionDef) != null)
                        Console.WriteLine("* Format Mode: PORTABLE (POSIX)");

                    var formatResult = result.GetOption(parserSchema.formatOptionDef);
                    if (formatResult != null)
                    {
                        // Retrieved by name and generic type
                        Console.WriteLine($"* Custom Format: {formatResult.GetArg<string>("FORMAT")}");
                    }

                    var outputResult = result.GetOption(parserSchema.outputOptionDef);
                    if (outputResult != null)
                    {
                        string mode = result.GetOption(parserSchema.appendOptionDef) is not null ? "Appending to" : "Overwriting";
                        // Retrieved by definition reference
                        Console.WriteLine($"* Output Route: {mode} file '{outputResult.GetArg(parserSchema.fileParamDef)}'");
                    }
                    else
                    {
                        Console.WriteLine("* Output Route: stderr");
                    }
                    Console.WriteLine("---------------------------------------\n");
                }
                else
                {
                    Console.WriteLine("Failed to parse command line. Exceptions:");
                    var ex = result.GetException()!;
                    Console.WriteLine($"- {ex.Message}");
                }

                Console.WriteLine("\nEnter another command line (or press Enter to quit):");
                line = Console.ReadLine()?.Split() ?? Array.Empty<string>();
            }
        }
    }
}