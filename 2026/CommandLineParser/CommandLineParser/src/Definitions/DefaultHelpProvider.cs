using System.Text;
using System.Linq;
using System.Collections.Generic;

namespace CommandLineParser
{
    internal class DefaultHelpProvider : HelpProvider
    {
        public DefaultHelpProvider(List<OptionDefinition> options, List<IParameterDefinition> plainArguments, HelpProvider programHelp)
            : base(programHelp.GetLongHelpText(), programHelp.GetShortHelpText())
        {
            _options = options;
            _plainArguments = plainArguments;
        }

        public override string GetLongHelpText()
        {
            EnsureGenerated();
            return _longText!;
        }

        public override string GetShortHelpText()
        {
            EnsureGenerated();
            return _shortText!;
        }

        private void EnsureGenerated()
        {
            if (_generated) return;
            GenerateHelp();
            _generated = true;
        }

        private void GenerateHelp()
        {
            int columnWidth = ComputeColumnWidth();

            var longBuilder = new StringBuilder();
            var shortBuilder = new StringBuilder();

            if (!string.IsNullOrWhiteSpace(base.GetLongHelpText()))
            {
                longBuilder.AppendLine(base.GetLongHelpText());
                longBuilder.AppendLine();
            }

            if (!string.IsNullOrWhiteSpace(base.GetShortHelpText()))
            {
                shortBuilder.AppendLine(base.GetShortHelpText());
                shortBuilder.AppendLine();
            }

            AppendOptions(longBuilder, shortBuilder, columnWidth);
            AppendArguments(longBuilder, shortBuilder, columnWidth);

            _longText = longBuilder.ToString().TrimEnd();
            _shortText = shortBuilder.ToString().TrimEnd();
        }

        private int ComputeColumnWidth()
        {
            int maxWidth = _options
                .Select(option =>
                {
                    int length = string.Join(", ", option.Aliases).Length;
                    if (option.ParameterDefinitions.Any())
                    {
                        length += 1 + string.Join(" ", option.ParameterDefinitions.Select(p => $"<{p.GetName()}>")).Length;
                    }
                    return length;
                })
                .DefaultIfEmpty(Constants.columnWidthBase)
                .Max();

            return Math.Max(Constants.columnWidthBase, maxWidth) + Constants.columnGap;
        }

        private void AppendOptions(StringBuilder longBuilder, StringBuilder shortBuilder, int columnWidth)
        {
            longBuilder.AppendLine("Options:");
            shortBuilder.AppendLine("Options:");

            var lines = _options
                .Select(option =>
                {
                    string aliases = string.Join(", ", option.Aliases);
                    string parameters = option.ParameterDefinitions.Any()
                        ? " " + string.Join(" ", option.ParameterDefinitions.Select(p => $"<{p.GetName()}>") )
                        : "";

                    string leftPart = $"  {aliases}{parameters}";
                    HelpProvider help = option.GetHelp();

                    return new
                    {
                        LongLine = $"{leftPart.PadRight(columnWidth)}{help.GetLongHelpText()}",
                        ShortLine = $"{leftPart.PadRight(columnWidth)}{help.GetShortHelpText()}"
                    };
                })
                .ToList();

            if (lines.Count == 0)
            {
                return;
            }

            longBuilder.AppendLine(string.Join(Environment.NewLine, lines.Select(line => line.LongLine)));
            shortBuilder.AppendLine(string.Join(Environment.NewLine, lines.Select(line => line.ShortLine)));
        }

        private void AppendArguments(StringBuilder longBuilder, StringBuilder shortBuilder, int columnWidth)
        {
            if (!_plainArguments.Any()) return;

            longBuilder.AppendLine();
            longBuilder.AppendLine("Arguments:");
            shortBuilder.AppendLine();
            shortBuilder.AppendLine("Arguments:");

            var lines = _plainArguments
                .Select(arg =>
                {
                    string leftPart = $"  <{arg.GetName()}>";
                    HelpProvider argHelp = arg.GetHelp();

                    return new
                    {
                        LongLine = $"{leftPart.PadRight(columnWidth)}{argHelp.GetLongHelpText()}",
                        ShortLine = $"{leftPart.PadRight(columnWidth)}{argHelp.GetShortHelpText()}"
                    };
                })
                .ToList();

            longBuilder.AppendLine(string.Join(Environment.NewLine, lines.Select(line => line.LongLine)));
            shortBuilder.AppendLine(string.Join(Environment.NewLine, lines.Select(line => line.ShortLine)));
        }

        private readonly List<OptionDefinition> _options;
        private readonly List<IParameterDefinition> _plainArguments;
        private bool _generated = false;
        private string? _longText;
        private string? _shortText;
    }
}
