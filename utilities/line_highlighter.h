#ifndef LINE_HIGHLIGHTER_H
#define LINE_HIGHLIGHTER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QMap>
#include <QDebug>
class line_highlighter : public QSyntaxHighlighter
{
  Q_OBJECT
public:
  explicit line_highlighter(QTextDocument *parent = nullptr);

  /**
   * @brief 设置字体颜色
   * @param key 关键字
   * @param color 颜色
   */
  void set_text_color(QString key, Qt::GlobalColor color)
  {
    key_map.insert(key, color);
  }

  /**
   * @brief 设置默认颜色
   * @param color 颜色
   */
  void set_default_color(Qt::GlobalColor color)
  {
    text_color = color;
  }

signals:

protected:
  /* 重写highlightBlock函数，设置每一行的字体颜色 */
  void highlightBlock(const QString &text) override
  {
    QTextCharFormat format;

    /* 获取当前行的文本 */
    QString line_text = text.trimmed();

    QMap<QString, Qt::GlobalColor>::const_iterator it = key_map.constBegin();
    while (it != key_map.constEnd())
    {
      QString key = it.key();
      Qt::GlobalColor color = it.value();
      ++it;
      if(line_text.contains(key))
      {
        /* 设置颜色 */
        format.setForeground(color);
        setFormat(0, line_text.length(), format);
        return;
      }
    }

    format.setForeground(text_color);
    setFormat(0, line_text.length(), format);
  }

private:
  QMap<QString, Qt::GlobalColor>key_map;
  Qt::GlobalColor text_color = Qt::white;
};

#endif // LINE_HIGHLIGHTER_H
