#!/bin/bash
# setup-github.sh - Setup GitHub remote for SimpleNotebook

# To create a Personal Access Token:
# 1. Go to https://github.com/settings/tokens
# 2. Click "Generate new token (classic)"
# 3. Give it a name, select "repo" scope
# 4. Copy the token

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO_DIR"

if [ -z "$1" ]; then
    echo "Usage: ./setup-github.sh <GITHUB_TOKEN>"
    echo ""
    echo "To create a Personal Access Token:"
    echo "1. Go to https://github.com/settings/tokens"
    echo "2. Click 'Generate new token (classic)'"
    echo "3. Give it a name, select 'repo' scope"
    echo "4. Copy the token and pass it as argument"
    exit 1
fi

TOKEN="$1"
REPO_NAME="SimpleNotebook"
USERNAME="RandumTom"

echo "Creating GitHub repository: $REPO_NAME"

# Create repo via GitHub API
RESPONSE=$(curl -s -X POST \
    -H "Authorization: token $TOKEN" \
    -H "Accept: application/vnd.github.v3+json" \
    https://api.github.com/user/repos \
    -d "{\"name\":\"$REPO_NAME\",\"description\":\"A simple desktop note-taking app for students\",\"private\":false}")

# Check if created or already exists
if echo "$RESPONSE" | grep -q '"html_url"'; then
    echo "Repository created successfully!"
    REPO_URL=$(echo "$RESPONSE" | grep '"html_url"' | cut -d'"' -f4)
elif echo "$RESPONSE" | grep -q '"Already exists"'; then
    echo "Repository already exists."
    REPO_URL="https://github.com/$USERNAME/$REPO_NAME"
else
    echo "Error creating repository:"
    echo "$RESPONSE"
    exit 1
fi

# Add remote and push
git remote remove origin 2>/dev/null
git remote add origin "https://github.com/$USERNAME/$REPO_NAME.git"
git branch -M main
git push -u origin main

echo ""
echo "Done! Your repo is at: $REPO_URL"
