FROM openjdk:11-jre-slim

WORKDIR /app

# 复制构建好的jar文件
COPY target/furbbs-backend-1.0.0.jar app.jar

# 暴露端口
EXPOSE 8080 8090

# 运行应用
CMD ["java", "-jar", "app.jar"]