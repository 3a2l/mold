#include "mold.h"

#include <tbb/global_control.h>

namespace COFF
{
    static
    std::vector<std::string_view> read_response_file(Context& ctx, std::string_view path)
    {
        std::vector<std::string_view> vec;
        MemoryMappedFile<E>* mb =
            MemoryMappedFile<E>::must_open(ctx, std::string(path));
        u8* data = mb->data(ctx);

        auto read_quoted = [&](i64 i, char quote) {
            std::string buf;
            while (i < mb->size() && data[i] != quote) {
                if (data[i] == '\\') {
                    buf.append(1, data[i + 1]);
                    i += 2;
                }
                else {
                    buf.append(1, data[i++]);
                }
            }
            if (i >= mb->size())
                Fatal(ctx) << path << ": premature end of input";
            vec.push_back(save_string(ctx, buf));
            return i + 1;
        };

        auto read_unquoted = [&](i64 i) {
            std::string buf;
            while (i < mb->size() && !isspace(data[i]))
                buf.append(1, data[i++]);
            vec.push_back(save_string(ctx, buf));
            return i;
        };

        for (i64 i = 0; i < mb->size();) {
            if (isspace(data[i]))
                i++;
            else if (data[i] == '\'')
                i = read_quoted(i + 1, '\'');
            else if (data[i] == '\"')
                i = read_quoted(i + 1, '\"');
            else
                i = read_unquoted(i);
        }
        return vec;
    }

    static
    std::vector<std::string_view> expand_response_files(Context& ctx, char* argv[])
    {
        std::vector<std::string_view> vec;

        for (int64_t i = 0; argv[i]; i++)
        {
            if (argv[i][0] == '@')
                append(vec, read_response_file(ctx, argv[i] + 1));
            else
                vec.push_back(argv[i]);
        }
        return vec;
    }

    static
    void parse_nonpositional_args(Context& ctx,
                                  std::vector<std::string_view>& remaining)
    {
        std::span<std::string_view> args = ctx.cmdline_args;
    }

    int do_main(int argc, char* argv[])
    {
        Context ctx;

        ctx.cmdline_args = expand_response_files(ctx, argv);
        std::vector<std::string_view> file_args;
        parse_nonpositional_args(ctx, file_args);

        tbb::global_control tbb_cont(tbb::global_control::max_allowed_parallelism,
                                     ctx.arg.thread_count);
    }
}
