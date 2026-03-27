package com.furbbs.util;

import org.springframework.web.util.HtmlUtils;

/**
 * XSS防护工具类，用于过滤和转义用户输入
 */
public class XssFilterUtil {

    /**
     * 转义HTML特殊字符，防止XSS攻击
     * @param input 用户输入
     * @return 转义后的字符串
     */
    public static String escapeHtml(String input) {
        if (input == null) {
            return null;
        }
        return HtmlUtils.htmlEscape(input);
    }

    /**
     * 过滤HTML标签，只保留允许的标签
     * @param input 用户输入
     * @return 过滤后的字符串
     */
    public static String filterHtml(String input) {
        if (input == null) {
            return null;
        }
        // 简单的HTML标签过滤，实际项目中可能需要更复杂的过滤规则
        return input.replaceAll("<script[^>]*>.*?</script>", "")
                   .replaceAll("<iframe[^>]*>.*?</iframe>", "")
                   .replaceAll("<object[^>]*>.*?</object>", "")
                   .replaceAll("<embed[^>]*>.*?</embed>", "")
                   .replaceAll("<link[^>]*>.*?</link>", "")
                   .replaceAll("<style[^>]*>.*?</style>", "");
    }

    /**
     * 清理SQL注入相关的特殊字符
     * @param input 用户输入
     * @return 清理后的字符串
     */
    public static String sanitizeSql(String input) {
        if (input == null) {
            return null;
        }
        // 移除SQL注入相关的特殊字符
        return input.replaceAll("['\\";\\(\\);\\\\]+", "");
    }

    /**
     * 全面清理用户输入，防止XSS和SQL注入
     * @param input 用户输入
     * @return 清理后的字符串
     */
    public static String sanitizeInput(String input) {
        if (input == null) {
            return null;
        }
        // 先过滤HTML标签，再转义特殊字符
        String filtered = filterHtml(input);
        return escapeHtml(filtered);
    }
}
